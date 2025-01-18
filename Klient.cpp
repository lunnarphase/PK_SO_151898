// Klient.cpp

#include "Klient.h"
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctime>
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

void Klient::dzialaj() {
    signal(SIGUSR2, obslugaSygnalu2);

    while (!sygnal2 && salonOtwarty) {
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

        // Sprawdz czy poczekalnia jest pelna
        struct sembuf sb = {0, -1, IPC_NOWAIT};

        if (semop(salonPtr->semidPoczekalnia, &sb, 1) == -1) {
            if (errno == EAGAIN) {
                cout << "Klient " << id << " opuszcza salon - brak miejsca w poczekalni." << endl;
                // Return to earning money
                sleep(rand() % 3 + 1);
                continue;
            } else {
                perror("Blad: semop wait on poczekalnia");
                exit(EXIT_FAILURE);
            }
        }

        // Wyslij wiadomosc do fryzjera o przybyciu
        Message msg;
        msg.mtype = MSG_TYPE_CLIENT_ARRIVAL;
        msg.clientId = id;

        if (msgsnd(msgid, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
            if (errno == EINTR) {
                continue; // Interrupted by a signal
            }
            perror("Blad: msgsnd");
            exit(EXIT_FAILURE);
        }

        // Po byciu obsluzonym i opuszczeniu poczekalni, zasygnalizuj semafor poczekalni 
        sb.sem_op = 1;
        if (semop(salonPtr->semidPoczekalnia, &sb, 1) == -1) {
            perror("Blad: semop signal on poczekalnia");
            exit(EXIT_FAILURE);
        }

        cout << "Klient " << id << " przybyl do salonu." << endl;

        sleep(5);

        if (sygnal2) {
            cout << "\nOdebrano sygnal 2. Klient " << id << " opuszcza salon." << endl;
            break;
        }

        if (!salonOtwarty) {
            cout << "Klient " << id << " opuszcza salon." << endl;
            break;
        }

        // Symulacja czasu miedzy wizytami klientow
        sleep(rand() % 3 + 1);
    }
}
