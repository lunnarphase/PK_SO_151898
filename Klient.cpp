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

extern bool salonOtwarty;
extern volatile sig_atomic_t sygnal2;

struct Message {
    long mtype;
    int clientId;
    int paymentAmount;
    int banknotes[10]; // Max 10 banknotow
    int numBanknotes;
    int pid; // PID klienta
};

const long MSG_TYPE_CLIENT_ARRIVAL = 1;

Klient::Klient(int id, Salon* salonPtr, Kasa* kasaPtr)
    : id(id), salonPtr(salonPtr), kasaPtr(kasaPtr), money(0) {
}

void Klient::dzialaj() {

    signal(SIGUSR2, obslugaSygnalu2);

    while (!sygnal2 && salonOtwarty) {

        // Klient zarabia pieniądze, aż uzbiera co najmniej 50 zł
        while (money < 50) {
            cout << "Klient " << id << " zarabia pieniadze. Aktualnie ma: " << money << " zl" << endl;
            sleep(1); // Symulacja godziny pracy
            money += 10;
            if (sygnal2 || !salonOtwarty) {
                break;
            }
        }
        if (sygnal2 || !salonOtwarty) {
            break;
        }

        key_t key = MSGQUEUE_KEY;
        int msgid = msgget(key, 0600 | IPC_CREAT);

        if (msgid == -1) {
            perror("Blad: msgget");
            exit(EXIT_FAILURE);
        }

        // Sprawdzenie, czy jest miejsce w poczekalni
        struct sembuf sb = {0, -1, IPC_NOWAIT};

        if (semop(salonPtr->semidPoczekalnia, &sb, 1) == -1) {
            if (errno == EAGAIN) {
                int cooldown = rand() % 8 + 3; // Losowy czas pracy od 3 do 10 sekund
                cout << "Klient " << id << " opuszcza salon - brak miejsca w poczekalni. Sproboje ponownie za " << cooldown << " sekund" << endl;
                sleep(cooldown);
                continue;
            } else {
                perror("Blad: semop - poczekalnia");
                exit(EXIT_FAILURE);
            }
        }

        // Przygotowanie płatności
        int payment = 30;
        vector<int> banknotes;

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

        // Display the banknotes used for payment
        cout << "Klient " << id << " przybył do salonu i zapłacił 30 zł - ";
        map<int, int> banknoteCount;
        for (int bn : banknotes) {
            banknoteCount[bn]++;
        }
        for (auto& pair : banknoteCount) {
            cout <<  "$" << pair.first << "x" << pair.second << " ";
        }
        cout << endl;
        sleep(1);

        // Dodanie banknotów do kasy
        for (int banknote : banknotes) {
            kasaPtr->dodajBanknot(banknote);
        }

        // Wysyłanie informacji do fryzjera o przybyciu
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

        sleep(1);

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

        // Zwolnienie miejsca w poczekalni
        struct sembuf sb_signal = {0, 1, 0};
        if (semop(salonPtr->semidPoczekalnia, &sb_signal, 1) == -1) {
            perror("Blad: semop signal on poczekalnia");
            exit(EXIT_FAILURE);
        }

        cout << "Klient " << id << " opuszcza salon i wraca do zarabiania pieniedzy." << endl;
        sleep(1);
    }
}
