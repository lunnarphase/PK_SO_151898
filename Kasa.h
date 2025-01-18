#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "KluczeIPC.h"

class Kasa {
public:
    int* banknoty10;
    int* banknoty20;
    int* banknoty50;

    int shmid;
    int semid;
    key_t shmkey;
    key_t semkey;

    Kasa();
    ~Kasa();

    void initSharedMemory();
    void initSemaphore();
    void removeSharedMemory();
    void removeSemaphore();

    void dodajBanknot(int nominal);
    bool wydajReszte(int reszta, int& wydane10, int& wydane20, int& wydane50);
};
