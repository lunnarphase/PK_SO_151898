// Fryzjer.cpp

#include "Fryzjer.h"
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ObslugaSygnalu.h"

using namespace std;

extern bool salonOtwarty;
extern volatile sig_atomic_t sygnal1;

struct Message {
    long mtype;
    int clientId;
};

const long MSG_TYPE_CLIENT_ARRIVAL = 1;

Fryzjer::Fryzjer(int id, Salon* salonPtr, Kasa* kasaPtr)
    : id(id), salonPtr(salonPtr), kasaPtr(kasaPtr) {
}

void Fryzjer::dzialaj() {
    signal(SIGUSR1, obslugaSygnalu1);
    signal(SIGUSR2, obslugaSygnalu2);

    key_t key = ftok("./salon_msgqueue", 80);
    if (key == -1) {
        perror("Blad: ftok");
        exit(EXIT_FAILURE);
    }
    int msgid = msgget(key, 0666 | IPC_CREAT);

    if (msgid == -1) {
        perror("Blad: msgget");
        exit(EXIT_FAILURE);
    }

    while (salonOtwarty) {
        Message msg;
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), MSG_TYPE_CLIENT_ARRIVAL, 0) == -1) {
            if (errno == EINTR) {
                continue; // Interrupted by a signal
            }
            perror("Blad: msgrcv");
            continue;
        }

        int klientId = msg.clientId;
        cout << "Fryzjer " << id << " obsluguje klienta " << klientId << "." << endl;

        sleep(2);

        // Obliczanie i wydawanie reszty
        int wydane10 = 0, wydane20 = 0, wydane50 = 0;
        int kwotaDoZwrotu = 20; // Przykładowa kwota do zwrotu

        while (!kasaPtr->wydajReszte(kwotaDoZwrotu, wydane10, wydane20, wydane50)) {
            cout << "Fryzjer " << id << " czeka, aż kasa będzie mogła wydać resztę dla klienta " << klientId << endl;
            sleep(1);
        }

        cout << "Fryzjer " << id << " wydaje reszty dla klienta " << klientId << ": " 
             << wydane10 << " x 10zl, " << wydane20 << " x 20zl, " << wydane50 << " x 50zl" << endl;

        if (sygnal1) {
            cout << "\nOdebrano sygnal 1. Fryzjer " << id << " konczy prace." << endl;
            break;
        }

        if (!salonOtwarty) {
            cout << "Fryzjer " << id << " konczy prace." << endl;
            break;
        }
    }
}
