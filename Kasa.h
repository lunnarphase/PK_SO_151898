#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "KluczeIPC.h"

class Kasa {
public:
    int* banknoty10; // Liczba banknotow o nominale 10 zl
    int* banknoty20; // Liczba banknotow o nominale 20 zl
    int* banknoty50; // Liczba banknotow o nominale 50 zl

    int shmid;       // Identyfikator pamieci wspoldzielonej
    int semid;       // Identyfikator semafora
    key_t shmkey;    // Klucz pamieci wspoldzielonej
    key_t semkey;    // Klucz semafora
 
    Kasa();
    ~Kasa();

    void initSharedMemory();    // Inicjalizacja pamieci wspoldzielonej
    void initSemaphore();       // Inicjalizacja semafora
    void removeSharedMemory();  // Usuniecie pamieci wspoldzielonej
    void removeSemaphore();     // Usuniecie semafora

    void dodajBanknot(int nominal);
    bool wydajReszte(int reszta, int& wydane10, int& wydane20, int& wydane50);
    void printBanknotes();
};
