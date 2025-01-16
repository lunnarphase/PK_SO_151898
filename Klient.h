
// Klient.h

#pragma once

#include <pthread.h>
#include <csignal> 

#include "Salon.h"
#include "Kasa.h"

class Klient {
public:
    int id;
    Salon* salonPtr;
    Kasa* kasaPtr;

    pthread_t th; // Obsluga watku

    Klient(int id, Salon* salonPtr, Kasa* kasaPtr);

    void start();
    void* dzialaj();

    static void* startThread(void* arg);
};
