
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
    shmkeyFotele = ftok("./salon_shmkey_fotele", 80); // Generacja unikalnego klucza
    if (shmkeyFotele == -1) {
        perror("Blad: ftok");
        exit(EXIT_FAILURE);
    }

    shmidFotele = shmget(shmkeyFotele, sizeof(int), 0666 | IPC_CREAT);
    if (shmidFotele == -1) {
        perror("Blad: shmget");
        exit(EXIT_FAILURE);
    }

    wolneFotele = (int*)shmat(shmidFotele, nullptr, 0);
    if (wolneFotele == (void*)-1) {
        perror("Blad: shmat");
        exit(EXIT_FAILURE);
    }

    *wolneFotele = *wolneFotele;
}

void Salon::initSemaphores() {
    // Initialize semaphore for chairs
    semkeyFotele = ftok("./salon_semkey_fotele", 81);
    if (semkeyFotele == -1) {
        perror("Blad: ftok for semkeyFotele");
        exit(EXIT_FAILURE);
    }

    semidFotele = semget(semkeyFotele, 1, 0666 | IPC_CREAT);
    if (semidFotele == -1) {
        perror("Blad: semget for semidFotele");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphore value to the number of chairs
    if (semctl(semidFotele, 0, SETVAL, *wolneFotele) == -1) {
        perror("Blad: semctl SETVAL for semidFotele");
        exit(EXIT_FAILURE);
    }

    // Similarly initialize semaphore for the waiting room
    semkeyPoczekalnia = ftok("./salon_semkey_poczekalnia", 82);
    if (semkeyPoczekalnia == -1) {
        perror("Blad: ftok for semkeyPoczekalnia");
        exit(EXIT_FAILURE);
    }

    semidPoczekalnia = semget(semkeyPoczekalnia, 1, 0666 | IPC_CREAT);
    if (semidPoczekalnia == -1) {
        perror("Blad: semget for semidPoczekalnia");
        exit(EXIT_FAILURE);
    }

    // Initialize semaphore value to the capacity of the waiting room
    if (semctl(semidPoczekalnia, 0, SETVAL, pojemnoscPoczekalni) == -1) {
        perror("Blad: semctl SETVAL for semidPoczekalnia");
        exit(EXIT_FAILURE);
    }
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
