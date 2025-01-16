
// Klient.cpp

#include "Klient.h"
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ObslugaSygnalu.h"

using namespace std;

extern bool salonOtwarty;
extern volatile sig_atomic_t sygnal2;

struct Message {
    long mtype;
    int clientId;
};

const long MSG_TYPE_CLIENT_ARRIVAL = 1;

Klient::Klient(int id, Salon* salonPtr, Kasa* kasaPtr)
    : id(id), salonPtr(salonPtr), kasaPtr(kasaPtr) {
}

void Klient::start() {
    int ret = pthread_create(&th, nullptr, Klient::startThread, this);
    if (ret != 0) {
        perror("Blad: Nie udalo sie utworzyc watku klienta");
    }
}

void* Klient::startThread(void* arg) {
    return ((Klient*)arg)->dzialaj();
}

void* Klient::dzialaj() {

    // Ustawienie obsługi sygnału
    signal(SIGUSR2, obslugaSygnalu2);

    while (!sygnal2 && salonOtwarty) {

        key_t key = ftok("salon_msgqueue", 80);
        int msgid = msgget(key, 0666 | IPC_CREAT);

        if (msgid == -1) {
            perror("Blad: msgget");
            exit(EXIT_FAILURE);
        }

        // Sprawdz czy poczekalnia jest pelna
        // Na ten moment zakladamy ze poczekalnia jest pelna

        Message msg;
        msg.mtype = MSG_TYPE_CLIENT_ARRIVAL;
        msg.clientId = id;

        if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        cout << "Klient " << id << " przybyl do salonu." << endl;

        sleep(5);

        if (sygnal2) {
            cout << "Klient " << id << " opuszcza salon ze wzgledu na sygnal 2." << endl;
            break;
        }

        if (!salonOtwarty) {
            cout << "Klient " << id << " opuszcza salon." << endl;
            break;
        }
    }

    return nullptr;
}
