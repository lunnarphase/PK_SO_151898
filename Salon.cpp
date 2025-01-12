#include "Salon.h"

Salon::Salon(int nFotele, int kPoczekalnia)
	: wolneFotele(nFotele), pojemnoscPoczekalni(kPoczekalnia) {}

Salon& Salon::operator=(const Salon& other) {
    if (this == &other) {
        return *this;
    }

    wolneFotele = other.wolneFotele;
    pojemnoscPoczekalni = other.pojemnoscPoczekalni;
    kolejkaKlientow = other.kolejkaKlientow;

    return *this;
}