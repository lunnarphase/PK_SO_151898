#pragma once

#include <csignal>
#include "Salon.h"
#include "Kasa.h"

class Fryzjer {
public:
    int id;
    Salon* salonPtr;
    Kasa* kasaPtr;

    Fryzjer(int id, Salon* salonPtr, Kasa* kasaPtr);

    void dzialaj();
};
