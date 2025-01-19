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
        // Klient zarabia pieniadze, aż uzbiera co najmniej 50 zł
        while (money < 50 && !sygnal2 && salonOtwarty) {
            cout << "\033[1;33mKlient " << id << " zarabia pieniadze. Aktualnie ma: " << money << " zl\033[0m" << endl;
            sleep(1);
            money += 10;
        }
        if (sygnal2 || !salonOtwarty) {
            break;
        }

        key_t key = MSGQUEUE_KEY;                  // Klucz kolejki komunikatów
        int msgid = msgget(key, 0600 | IPC_CREAT); // Utworzenie kolejki komunikatów

        if (msgid == -1) { // Obsługa błędu utworzenia kolejki komunikatów
            perror("Blad: msgget");
            exit(EXIT_FAILURE);
        }

        // Sprawdzenie, czy jest miejsce w poczekalni
        struct sembuf sb = {0, -1, IPC_NOWAIT};

        // Próba zajęcia miejsca w poczekalni
        if (semop(salonPtr->semidPoczekalnia, &sb, 1) == -1) 
        {
            // Jeśli brakuje miejsca w poczekalni
            if (errno == EAGAIN) { 
                int cooldown = rand() % 8 + 3; // Czas oczekiwania na ponowną próbę
                cout << "Klient " << id << " opuszcza salon - brak miejsca w poczekalni. Sproboje ponownie za " << cooldown << " sekund" << endl;
                sleep(cooldown);
                continue;
            } else {
                perror("Blad: semop - poczekalnia");
                exit(EXIT_FAILURE);
            }
        }

        // Pobranie aktualnej wartości semafora poczekalni
        int currPoczekalniaVal = semctl(salonPtr->semidPoczekalnia, 0, GETVAL);
        if (currPoczekalniaVal == -1) {
            perror("Blad: semctl GETVAL for semidPoczekalnia");
            exit(EXIT_FAILURE);
        }

        int occupiedWaitingSeats = salonPtr->pojemnoscPoczekalni - currPoczekalniaVal; // Liczba zajętych miejsc w poczekalni
        cout << "Klient " << id << " udaje sie do poczekalni - aktualny stan poczekalni: " << occupiedWaitingSeats << " / " << salonPtr->pojemnoscPoczekalni << endl;

        // Przygotowanie płatności
        int payment = 30;        // Koszt usługi
        int maxPayment = 50;     // Maksymalna kwota płatności

        // Obliczenie liczby możliwych kwot płatności (30, 40, 50)
        int amountOptions = (maxPayment - payment) / 10 + 1;
        // Losowy indeks z zakresu 0 do amountOptions - 1
        int randomIndex = rand() % amountOptions;
        // Obliczenie kwoty płatności
        int totalPaymentAmount = payment + randomIndex * 10; // Kwoty: 30, 40, 50

        vector<int> banknotes;   // Banknoty użyte do płatności
        int totalBanknotesAmount = 0;

        while (totalBanknotesAmount < totalPaymentAmount) {
            int banknote;
            int randChoice = rand() % 3; // Losowo wybieramy 0, 1 lub 2

            if (randChoice == 0) {
                banknote = 10;
            } else if (randChoice == 1) {
                banknote = 20;
            } else {
                banknote = 50;
            }

            // Sprawdzamy, czy dodanie tego banknotu nie przekroczy żądanej kwoty
            if (totalBanknotesAmount + banknote > totalPaymentAmount) {
                continue; // Pomijamy ten banknot i losujemy ponownie
            }

            banknotes.push_back(banknote);
            totalBanknotesAmount += banknote;
        }

        // Aktualizacja pieniędzy klienta
        money -= totalBanknotesAmount;

        // Wyświetlenie informacji o płatności
        cout << "Klient " << id << " przygotował płatność - ";
        map<int, int> banknoteCount;
        for (int bn : banknotes) {
            banknoteCount[bn]++;
        }
        for (auto& pair : banknoteCount) {
            cout << "$" << pair.first << "x" << pair.second << " ";
        }
        cout << "(Łącznie: " << totalBanknotesAmount << " zł)" << endl;
        sleep(1);

        // Wysłanie wiadomości do fryzjera
        Message msg;
        msg.mtype = MSG_TYPE_CLIENT_ARRIVAL;
        msg.clientId = id;
        msg.paymentAmount = totalBanknotesAmount;  // Faktyczna kwota przekazana fryzjerowi
        msg.numBanknotes = banknotes.size();
        for (int i = 0; i < banknotes.size(); ++i) {
            msg.banknotes[i] = banknotes[i];
        }
        msg.pid = getpid(); // Dodanie PID klienta

        if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("Błąd: msgsnd");
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

        // Zwolnienie miejsca w poczekalni po obsłudze
        struct sembuf sb_signal = {0, 1, 0};
        if (semop(salonPtr->semidPoczekalnia, &sb_signal, 1) == -1) {
            perror("Blad: semop signal on poczekalnia");
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