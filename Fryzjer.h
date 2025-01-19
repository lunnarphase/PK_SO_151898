#pragma once

#include <csignal>
#include "Salon.h"
#include "Kasa.h"

class Fryzjer {
public:
    int id;          // Identyfikator fryzjera
    Salon* salonPtr; // Wskaźnik do salonu
    Kasa* kasaPtr;   // Wskaźnik do kasy

    Fryzjer(int id, Salon* salonPtr, Kasa* kasaPtr);

    void dzialaj();
};
