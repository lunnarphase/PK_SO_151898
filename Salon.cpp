
// Salon.cpp

#include "Salon.h"
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

Salon::Salon(int nFotele, int kPoczekalnia)
    : pojemnoscPoczekalni(kPoczekalnia) {

    wolneFotele = new int;
    *wolneFotele = nFotele;

    if (pthread_mutex_init(&mtxPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji mtxPoczekalnia");
    }
    if (pthread_mutex_init(&mtxFotele, nullptr) != 0) {
        perror("Blad inicjalizacji mtxFotele");
    }
    if (pthread_cond_init(&cvPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji cvPoczekalnia");
    }
}

Salon::~Salon() {
    pthread_mutex_destroy(&mtxPoczekalnia);
    pthread_mutex_destroy(&mtxFotele);
    pthread_cond_destroy(&cvPoczekalnia);

    delete wolneFotele;
}

void Salon::initSharedMemory() {
    shmkeyFotele = ftok("salon_shmkey_fotele", 80); // Generacja unikalnego klucza
    if (shmkeyFotele == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    shmidFotele = shmget(shmkeyFotele, sizeof(int), 0666 | IPC_CREAT);
    if (shmidFotele == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    wolneFotele = (int*)shmat(shmidFotele, nullptr, 0);
    if (wolneFotele == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    *wolneFotele = *wolneFotele;
}

void Salon::initSemaphores() {
    // Mozliwa implementacja w przyszlosci - funckja sluzaca inicjalizacji semaforow
}

void Salon::removeSharedMemory() {
    if (shmdt(wolneFotele) == -1) {
        perror("Blad: shmdt wolneFotele");
    }

    if (shmctl(shmidFotele, IPC_RMID, nullptr) == -1) {
        perror("Blad: shmctl shmidFotele");
    }
}

void Salon::removeSemaphores() {
    // Mozliwa implementacja w przyszlosci - usuniecie semaforow
}

Salon& Salon::operator=(const Salon& other) {
    if (this == &other) {
        return *this;
    }

    delete wolneFotele;

    wolneFotele = new int;
    *wolneFotele = *other.wolneFotele;
    pojemnoscPoczekalni = other.pojemnoscPoczekalni;
    kolejkaKlientow = other.kolejkaKlientow;

    // Reinicjalizacja muteksow i zmiennych warunkowych
    pthread_mutex_destroy(&mtxPoczekalnia);
    pthread_mutex_destroy(&mtxFotele);
    pthread_cond_destroy(&cvPoczekalnia);

    if (pthread_mutex_init(&mtxPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji mtxPoczekalnia w operator=");
    }
    if (pthread_mutex_init(&mtxFotele, nullptr) != 0) {
        perror("Blad inicjalizacji mtxFotele w operator=");
    }
    if (pthread_cond_init(&cvPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji cvPoczekalnia w operator=");
    }

    return *this;
}
