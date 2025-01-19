#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "Fryzjer.h"
#include "ObslugaSygnalu.h"
#include "KluczeIPC.h"

using namespace std;

extern bool salonOtwarty;
extern volatile sig_atomic_t sygnal1;
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

Fryzjer::Fryzjer(int id, Salon* salonPtr, Kasa* kasaPtr)
    : id(id), salonPtr(salonPtr), kasaPtr(kasaPtr) {
}

void Fryzjer::dzialaj() {
    signal(SIGUSR1, obslugaSygnalu1);
    signal(SIGUSR2, obslugaSygnalu2);

    key_t key = MSGQUEUE_KEY;
    int msgid = msgget(key, 0600 | IPC_CREAT);

    if (msgid == -1) {
        perror("Blad: msgget");
        exit(EXIT_FAILURE);
    }

    while (salonOtwarty && !sygnal2) {
        Message msg;
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), MSG_TYPE_CLIENT_ARRIVAL, 0) == -1) {
            if (errno == EINTR) {
                if (sygnal1) {
                    cout << "Fryzjer " << id << " konczy prace ze wzgledu na sygnal 1" << endl;
                    sleep(1);
                    break;
                }
                continue;
            }
            perror("Blad: msgrcv");
            continue;
        }

        int klientId = msg.clientId;
        int payment = msg.paymentAmount;
        int klientPid = msg.pid; // PID klienta

        // Zajmowanie fotela
        struct sembuf sb_fotel = {0, -1, 0};
        if (semop(salonPtr->semidFotele, &sb_fotel, 1) == -1) {
            if (errno == EINTR) {
                if (sygnal2 || !salonOtwarty) {
                    cout << "Fryzjer " << id << " konczy prace ze wzgledu na sygnal 1" << endl;
                    break;
                }
                continue;
            } else {
                perror("Blad: semop - zajmowanie fotela");
                exit(EXIT_FAILURE);
            }
        }

        // Symulacja obsługi klienta przez określony czas
        cout << "\033[1;34mFryzjer " << id << " obsługuje klienta " << klientId << ".\033[0m" << endl;
        int czasObslugi = 3; // Czas obsługi w sekundach
        int czasSpedzony = 0;
        while (czasSpedzony < czasObslugi && !sygnal2) {
            sleep(1); // Symulowanie 1 sekundy obsługi
            czasSpedzony++;
        }
        if (sygnal2) {
            cout << "Fryzjer " << id << " przerywa obsługę klienta " << klientId << " z powodu sygnału 2." << endl;
            cout << "Klient " << klientId << " opuszcza salon." << endl;
        }

        // Wydawanie reszty
        int reszta = payment - 30;
        while (true) {
            int wydane10 = 0, wydane20 = 0, wydane50 = 0;
            if (kasaPtr->wydajReszte(reszta, wydane10, wydane20, wydane50)) {
                cout << "Fryzjer " << id << " wydaje klientowi " << klientId << " reszte: " << reszta << " zl"
                     << " - $10x" << wydane10 << " " << "$20x" << wydane20 << " " << "$50x" << wydane50 << endl;
                sleep(1);
                break;
            } else {
                cout << "Fryzjer " << id << " czeka na srodki w kasie, aby wydac reszte klientowi " << klientId << endl;
                sleep(1);
            }
        }

        // Wysłanie wiadomości do klienta o zakończeniu usługi
        Message responseMsg;
        responseMsg.mtype = klientPid; // Używamy PID klienta jako typ wiadomości
        responseMsg.clientId = klientId;
        responseMsg.paymentAmount = reszta; // Kwota reszty
        responseMsg.pid = getpid(); // PID fryzjera

        if (msgsnd(msgid, &responseMsg, sizeof(Message) - sizeof(long), 0) == -1) {
            perror("Blad: msgsnd do klienta");
            exit(EXIT_FAILURE);
        }

        // Zwalnianie fotela
        struct sembuf sb_fotel_release = {0, 1, 0};
        if (semop(salonPtr->semidFotele, &sb_fotel_release, 1) == -1) {
            perror("Blad: semop signal on fotele");
            exit(EXIT_FAILURE);
        }

        if (sygnal1) {
            cout << "\nOdebrano sygnal 1. Fryzjer " << id << " konczy prace" << endl;
            sleep(1);
            break;
        }
    }
}
