
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

struct Message {
    long mtype;
    int clientId;
};

const long MSG_TYPE_CLIENT_ARRIVAL = 1;

Fryzjer::Fryzjer(int id, Salon* salonPtr, Kasa* kasaPtr)
    : id(id), salonPtr(salonPtr), kasaPtr(kasaPtr) {
}

void Fryzjer::start() {
    int ret = pthread_create(&th, nullptr, Fryzjer::startThread, this);
    if (ret != 0) {
        perror("Nie udalo sie utworzyc watku fryzjera");
    }
}

void* Fryzjer::startThread(void* arg) {
    return ((Fryzjer*)arg)->dzialaj();
}

void* Fryzjer::dzialaj() {
    signal(SIGUSR2, obslugaSygnalu2);

    key_t key = ftok("salon_msgqueue", 80);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    if (msgid == -1) {
        perror("Blad: msgget");
        exit(EXIT_FAILURE);
    }

    while (salonOtwarty) {
        Message msg;
        if (msgrcv(msgid, &msg, sizeof(Message) - sizeof(long), MSG_TYPE_CLIENT_ARRIVAL, 0) == -1) {
            perror("Blad: msgrcv");
            continue;
        }

        int klientId = msg.clientId;
        cout << "Fryzjer " << id << " obsluguje klienta " << klientId << "." << endl;

        sleep(2);

        cout << "Fryzjer " << id << " wydaje reszte klientowi " << klientId << "." << endl;

        if (sygnal1) {
            cout << "Fryzjer " << id << " konczy prace po obsludze klienta." << endl;
            break;
        }

        if (!salonOtwarty) {
            cout << "Fryzjer " << id << " konczy prace." << endl;
            break;
        }
    }

    return nullptr;
}
