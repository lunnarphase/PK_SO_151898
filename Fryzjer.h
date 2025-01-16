
// Fryzjer.h

#pragma once

#include <pthread.h>
#include <csignal>

#include "Salon.h"
#include "Kasa.h"

class Fryzjer {
public:
    int id;
    Salon* salonPtr;
    Kasa* kasaPtr;

    pthread_t th;  // Obsluga sygnalu

    Fryzjer(int id, Salon* salonPtr, Kasa* kasaPtr);

    void start();
    void* dzialaj();

    static void* startThread(void* arg);
};
