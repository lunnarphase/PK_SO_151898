#include "Salon.h"
#include <cstring>
#include <cerrno>
#include <cstdio>

Salon::Salon(int nFotele, int kPoczekalnia)
    : wolneFotele(nFotele), pojemnoscPoczekalni(kPoczekalnia) {
    if (pthread_mutex_init(&mtxPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji mtxPoczekalnia");
    }
    if (pthread_mutex_init(&mtxFotele, nullptr) != 0) {
        perror("Blad inicjalizacji mtxFotele");
    }
    if (pthread_cond_init(&cvPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji cvPoczekalnia");
    }
}

Salon::~Salon() {
    pthread_mutex_destroy(&mtxPoczekalnia);
    pthread_mutex_destroy(&mtxFotele);
    pthread_cond_destroy(&cvPoczekalnia);
}

Salon& Salon::operator=(const Salon& other) { // Implementacja kopiowania danych i inicjalizacja muteksow
    if (this == &other) {
        return *this;
    }

    wolneFotele = other.wolneFotele;
    pojemnoscPoczekalni = other.pojemnoscPoczekalni;
    kolejkaKlientow = other.kolejkaKlientow;

    if (pthread_mutex_init(&mtxPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji mtxPoczekalnia w operator=");
    }
    if (pthread_mutex_init(&mtxFotele, nullptr) != 0) {
        perror("Blad inicjalizacji mtxFotele w operator=");
    }
    if (pthread_cond_init(&cvPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji cvPoczekalnia w operator=");
    }

    return *this;
}
