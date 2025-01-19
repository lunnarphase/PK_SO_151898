#include <vector>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <map>

#include "Klient.h"
#include "ObslugaSygnalu.h"
#include "KluczeIPC.h"

using namespace std;

extern bool salonOtwarty;               // Zmienna globalna informujaca o stanie salonu
extern volatile sig_atomic_t sygnal2;   // Zmienna globalna informujaca o sygnale 2

struct Message 
{
    long mtype;         // Typ wiadomosci
    int clientId;       // Identyfikator klienta
    int paymentAmount;  // Kwota platnosci
    int banknotes[10];  // Max 10 banknotow
    int numBanknotes;   // Liczba banknotow
    int pid;            // PID klienta
};

const long MSG_TYPE_CLIENT_ARRIVAL = 1; // Typ wiadomosci o przybyciu klienta

Klient::Klient(int id, Salon* salonPtr, Kasa* kasaPtr) : id(id), salonPtr(salonPtr), kasaPtr(kasaPtr), money(0) {}

void Klient::dzialaj() 
{
    signal(SIGUSR2, obslugaSygnalu2);

    while (!sygnal2 && salonOtwarty) 
    {
        // Klient zarabia pieniadze, az uzbiera co najmniej 50 zl
        while (money < 50 && !sygnal2 && salonOtwarty) {
            cout << "\033[1;33mKlient " << id << " zarabia pieniadze. Aktualnie ma: " << money << " zl\033[0m" << endl;
            sleep(1);
            money += 10;
        }
        if (sygnal2 || !salonOtwarty) {
            break;
        }

        key_t key = MSGQUEUE_KEY;                  // Klucz kolejki komunikatow
        int msgid = msgget(key, 0600 | IPC_CREAT); // Utworzenie kolejki komunikatow

        if (msgid == -1) { // Obsluga bledu utworzenia kolejki komunikatow
            perror("Blad: msgget");
            exit(EXIT_FAILURE);
        }

        // Sprawdzenie, czy jest miejsce w poczekalni
        struct sembuf sb = {0, -1, IPC_NOWAIT};

        // Proba zajecia miejsca w poczekalni
        if (semop(salonPtr->semidPoczekalnia, &sb, 1) == -1) 
        {
            // Jesli brakuje miejsca w poczekalni
            if (errno == EAGAIN) { 
                int cooldown = rand() % 8 + 3; // Czas oczekiwania na ponowna probe
                cout << "Klient " << id << " opuszcza salon - brak miejsca w poczekalni. Sproboje ponownie za " << cooldown << " sekund" << endl;
                sleep(cooldown);
                continue;
            } else {
                perror("Blad: semop - poczekalnia");
                exit(EXIT_FAILURE);
            }
        }

        // Pobrane aktualnej wartości semafora poczekalni
        int currPoczekalniaVal = semctl(salonPtr->semidPoczekalnia, 0, GETVAL);
        if (currPoczekalniaVal == -1) {
            perror("Blad: semctl GETVAL for semidPoczekalnia");
            exit(EXIT_FAILURE);
        }

        int occupiedWaitingSeats = salonPtr->pojemnoscPoczekalni - currPoczekalniaVal; // Pobranie liczby zajetych miejsc w poczekalni
        cout << "Klient " << id << " udaje sie do poczekalni - aktualny stan poczekalni: " << occupiedWaitingSeats << " / " << salonPtr->pojemnoscPoczekalni << endl;

        // ## Przygotowanie płatności ##
        int payment = 30;        // Kwota platnosci
        vector<int> banknotes;   // Banknoty uzyte do platnosci
        int remainingAmount = payment;
        while (remainingAmount > 0) {
            int banknote = 0;
            int randChoice = rand() % 2;
            if (remainingAmount >= 20 && randChoice == 0) {
                banknote = 20;
            } else {
                banknote = 10;
            }
            banknotes.push_back(banknote);
            remainingAmount -= banknote;
        }

        money -= payment;

        // Wyswietlenie informacji o ilosci i wartosci uzytych banknotow
        cout << "Klient " << id << " przygotowal platnosc - ";
        map<int, int> banknoteCount;
        for (int bn : banknotes) {
            banknoteCount[bn]++;
        }
        for (auto& pair : banknoteCount) {
            cout <<  "$" << pair.first << "x" << pair.second << " ";
        }
        cout << endl;
        sleep(1);

        // Wyslanie wiadomosci do fryzjera
        Message msg;
        msg.mtype = MSG_TYPE_CLIENT_ARRIVAL;
        msg.clientId = id;
        msg.paymentAmount = payment;
        msg.numBanknotes = banknotes.size();

        for (int i = 0; i < banknotes.size(); ++i) {
            msg.banknotes[i] = banknotes[i];
        }
        msg.pid = getpid(); // Dodanie PID klienta

        if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("Blad: msgsnd");
            exit(EXIT_FAILURE);
        }

        // Zwolnienie miejsca w poczekalni przed obsługą
        struct sembuf sb_signal = {0, 1, 0};
        if (semop(salonPtr->semidPoczekalnia, &sb_signal, 1) == -1) {
            perror("Blad: semop signal on poczekalnia");
            exit(EXIT_FAILURE);
        }

        // Oczekiwanie na rozpoczęcie usługi (fryzjer odbierze wiadomość)
        cout << "Klient " << id << " czeka na wolny fotel..." << endl;

        // Oczekiwanie na zakończenie usługi i wydanie reszty
        Message responseMsg;
        if (msgrcv(msgid, &responseMsg, sizeof(Message) - sizeof(long), getpid(), 0) == -1) {
            if (errno == EINTR) {
                if (sygnal2 || !salonOtwarty) {
                    break;
                }
                continue;
            }
            perror("Blad: msgrcv od fryzjera");
            exit(EXIT_FAILURE);
        }

        int reszta = responseMsg.paymentAmount;
        cout << "Klient " << id << " otrzymal reszte: " << reszta << " zl." << endl;
        sleep(1);

        money += reszta;

        cout << "Klient " << id << " opuszcza salon i wraca do zarabiania pieniedzy." << endl;
        sleep(1);
    }
}