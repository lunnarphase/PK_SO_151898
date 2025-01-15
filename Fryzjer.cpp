#include "Fryzjer.h"
#include "Salon.h"
#include "Kasa.h"
#include <unistd.h>
#include <cstdio>

using namespace std;

extern Salon salon;
extern Kasa kasa;
extern volatile sig_atomic_t sygnal1;
extern bool salonOtwarty;

Fryzjer::Fryzjer(int id) : id(id) {}

void Fryzjer::start() {
    int ret = pthread_create(&th, nullptr, &Fryzjer::dzialaj, this);
    if (ret != 0) {
        perror("Nie udalo sie utworzyc watku fryzjera");
    }
}

void* Fryzjer::dzialaj(void* arg) {
    Fryzjer* self = static_cast<Fryzjer*>(arg);

    while (true) {
        if (sygnal1 || !salonOtwarty) { // sprawdzenie czy salon jest otwarty
            cout << "Fryzjer " << self->id << " konczy prace." << endl;
            break;
        }

        // Blokujemy muteks poczekalni
        if (pthread_mutex_lock(&salon.mtxPoczekalnia) != 0) {
            perror("Blad przy zablokowaniu mtxPoczekalnia");
            continue;
        }

        while (salon.kolejkaKlientow.empty()) {
            cout << "Fryzjer " << self->id << " czeka na klienta." << endl;

            if (sygnal1) {
                pthread_mutex_unlock(&salon.mtxPoczekalnia);
                break;
            }

            // Czekamy na klienta
            pthread_cond_wait(&salon.cvPoczekalnia, &salon.mtxPoczekalnia);
        }

        if (sygnal1) {
            pthread_mutex_unlock(&salon.mtxPoczekalnia);
            break;
        }

        int klientId = salon.kolejkaKlientow.front();
        salon.kolejkaKlientow.pop();
        if (pthread_mutex_unlock(&salon.mtxPoczekalnia) != 0) {
            perror("Blad przy odblokowaniu mtxPoczekalnia");
        }

        // Znajdujemy wolny fotel
        if (pthread_mutex_lock(&salon.mtxFotele) != 0) {
            perror("Blad przy zablokowaniu mtxFotele");
            continue;
        }

        if (salon.wolneFotele > 0) {
            salon.wolneFotele--;
            cout << "Fryzjer " << self->id << " obsluguje klienta " << klientId << "." << endl;
        }
        else {
            cout << " -> Brak wolnych foteli!" << endl;
            pthread_mutex_unlock(&salon.mtxFotele);
            continue;
        }

        if (pthread_mutex_unlock(&salon.mtxFotele) != 0) {
            perror("Blad przy odblokowaniu mtxFotele");
        }

        // Symulacja pobierania oplaty i umieszczania w kasie
        int oplata = 50;
        int banknotKlienta = 50;
        kasa.dodajBanknot(banknotKlienta);

        // Realizacja uslugi
        sleep(2);

        // Zwolnienie fotela
        if (pthread_mutex_lock(&salon.mtxFotele) != 0) {
            perror("Blad przy zablokowaniu mtxFotele");
            continue;
        }
        salon.wolneFotele++;
        if (pthread_mutex_unlock(&salon.mtxFotele) != 0) {
            perror("Blad przy odblokowaniu mtxFotele");
        }

        // Wyliczenie reszty i pobieranie z kasy
        int reszta = banknotKlienta - oplata;
        int wydane10 = 0, wydane20 = 0, wydane50 = 0;

        if (reszta > 0) {
            bool resztaWydana = kasa.wydajReszte(reszta, wydane10, wydane20, wydane50);

            if (resztaWydana) {
                cout << "Fryzjer " << self->id << " wydaje reszte klientowi " << klientId << "." << endl;
            }
            else {
                cout << "Wystapil blad w trakcie wydawania reszty" << endl;
            }
        }

        // Tutaj powinna nastapic komunikacja z klientem o zakonczeniu uslugi i wydaniu reszty (do zaimplementowania)

        if (sygnal1) {
            cout << "Fryzjer " << self->id << " konczy prace po obsludze klienta." << endl;
            break;
        }

        if (!salonOtwarty) {
            cout << "Fryzjer " << self->id << " konczy prace." << endl;
            break;
        }
    }

    return nullptr;
}
