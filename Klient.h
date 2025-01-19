#pragma once

#include <csignal>
#include "Salon.h"
#include "Kasa.h"

class Klient {
public:
    int id;             // Identyfikator klienta
    int money;          // Aktualna ilość pieniędzy klienta
    Salon* salonPtr;    // Wskaźnik do salonu
    Kasa* kasaPtr;      // Wskaźnik do kasy

    Klient(int id, Salon* salonPtr, Kasa* kasaPtr);

    void dzialaj();
};
