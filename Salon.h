
// Salon.h

#pragma once

#include <queue>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

using namespace std;

class Salon {
public:
    int* wolneFotele;
    int* poczekalniaKlienci; // Wskaznik do pamieci wspoldzielonej reprezentujacej klientow w poczekalni

    int shmidFotele;
    int shmidPoczekalnia;
    key_t shmkeyFotele;
    key_t shmkeyPoczekalnia;

    int semidFotele;
    int semidPoczekalnia;
    key_t semkeyFotele;
    key_t semkeyPoczekalnia;

    int pojemnoscPoczekalni;

    pthread_mutex_t mtxPoczekalnia;
    pthread_mutex_t mtxFotele;
    pthread_cond_t  cvPoczekalnia;

    queue<int> kolejkaKlientow;

    Salon(int nFotele, int kPoczekalnia);
    ~Salon();

    void initSharedMemory();
    void initSemaphores();
    void removeSharedMemory();
    void removeSemaphores();

    Salon& operator=(const Salon& other);
};
