// Klient.h

#pragma once

#include <csignal>
#include "Salon.h"
#include "Kasa.h"

class Klient {
public:
    int id;
    int money; // Aktualna ilość pieniędzy klienta
    Salon* salonPtr;
    Kasa* kasaPtr;

    Klient(int id, Salon* salonPtr, Kasa* kasaPtr);

    void dzialaj();
};
