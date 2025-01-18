#pragma once

#include <queue>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "KluczeIPC.h"

class Salon {
public:
    int* wolneFotele;
    int pojemnoscPoczekalni;
    int liczbaFoteli;

    int shmidFotele;
    int semidFotele;
    int semidPoczekalnia;

    pthread_mutex_t mtxPoczekalnia;
    pthread_mutex_t mtxFotele;
    pthread_cond_t  cvPoczekalnia;

    Salon(int nFotele, int kPoczekalnia);
    ~Salon();

    void initSharedMemory();
    void initSemaphores();
    void removeSharedMemory();
    void removeSemaphores();

    Salon& operator=(const Salon& other);
};
