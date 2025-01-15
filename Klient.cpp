#include "Klient.h"
#include "Salon.h"
#include <unistd.h>
#include <cstdio>

extern Salon salon;
extern volatile sig_atomic_t sygnal2;
extern bool salonOtwarty;

Klient::Klient(int id)
    : id(id) {
}

void Klient::start() {
    int ret = pthread_create(&th, nullptr, &Klient::dzialaj, this);
    if (ret != 0) {
        perror("Nie udalo sie utworzyc watku klienta");
    }
}

void* Klient::dzialaj(void* arg) {
    Klient* self = static_cast<Klient*>(arg);

    while (!sygnal2) {

        if (!salonOtwarty) {
            cout << "Klient " << self->id << " nie moze wejsc do salonu - salon jest zamkniety." << endl;
            break;
        }

        sleep(rand() % 5 + 1); // Symulacja zarabiania pieniedzy

        // Blokujemy muteks poczekalni
        if (pthread_mutex_lock(&salon.mtxPoczekalnia) != 0) {
            perror("Blad przy probie zablokowania mtxPoczekalnia");
            continue;
        }

        if (salon.kolejkaKlientow.size() < salon.pojemnoscPoczekalni) {
            salon.kolejkaKlientow.push(self->id);
            cout << "Klient " << self->id << " czeka w poczekalni." << endl;

            // Powiadamiamy fryzjera
            if (pthread_cond_signal(&salon.cvPoczekalnia) != 0) {
                perror("Blad przy signal cvPoczekalnia");
            }
        }
        else {
            // Brak miejsca w poczekalni
            cout << "Klient " << self->id << " opuszcza salon z powodu braku miejsca." << endl;
            pthread_mutex_unlock(&salon.mtxPoczekalnia);
            continue;
        }

        // Odblokowujemy muteks poczekalni
        if (pthread_mutex_unlock(&salon.mtxPoczekalnia) != 0) {
            perror("Blad przy odblokowaniu mtxPoczekalnia");
        }

        // Tutaj klient powinien czeka? na obs?ug? i otrzymanie reszty (do zaimplementowania w kolejnych krokach)

        if (sygnal2) {
            cout << "Klient " << self->id << " opuszcza salon ze wzgledu na sygnal 2." << endl;
            break;
        }

        // Sprawdzamy, czy salon nie zosta? zamkni?ty podczas czekania
        if (!salonOtwarty) {
            cout << "Klient " << self->id << " opuszcza salon." << endl;
            break;
        }
    }

    return nullptr;
}
