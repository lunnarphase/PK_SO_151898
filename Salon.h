#pragma once

#include <queue>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "KluczeIPC.h"

class Salon {
public:
    int* wolneFotele;           // Liczba wolnych foteli
    int pojemnoscPoczekalni;    // Pojemnosc poczekalni
    int liczbaFoteli;           // Liczba foteli w salonie

    int shmidFotele;        // Identyfikator pamieci wspoldzielonej dla foteli
    int semidFotele;        // Identyfikator semafora dla foteli
    int semidPoczekalnia;   // Identyfikator semafora dla poczekalni

    pthread_mutex_t mtxPoczekalnia; // Mutex dla poczekalni - dostep do zmiennej pojemnoscPoczekalni
    pthread_mutex_t mtxFotele;      // Mutex dla foteli - dostep do zmiennej wolneFotele
    pthread_cond_t  cvPoczekalnia;  // Zmienna warunkowa dla poczekalni

    Salon(int nFotele, int kPoczekalnia);
    ~Salon();

    void initSharedMemory();
    void initSemaphores();
    void removeSharedMemory();
    void removeSemaphores();

    Salon& operator=(const Salon& other); // Przeciazenie operatora przypisania
};
