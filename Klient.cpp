#include "Klient.h"
#include "Salon.h"

extern Salon salon;
extern atomic<bool> sygnal2;
extern atomic<bool> salonOtwarty;

Klient::Klient(int id)
    : id(id) {
}

void Klient::start() {
    th = thread(&Klient::dzialaj, this);
}

void Klient::dzialaj() const
{
    while (!sygnal2) {

        if (!salonOtwarty) {
            cout << "Klient " << id << " nie moze wejsc do salonu - salon jest zamkniety." << endl;
            break;
        }

        this_thread::sleep_for(chrono::seconds(rand() % 5 + 1));

        this_thread::sleep_for(chrono::seconds(rand() % 5 + 1));

        unique_lock<mutex> lockPoczekalnia(salon.mtxPoczekalnia);

        if (salon.kolejkaKlientow.size() < salon.pojemnoscPoczekalni) {
            salon.kolejkaKlientow.push(id);
            cout << "Klient " << id << " czeka w poczekalni." << endl;
            salon.cvPoczekalnia.notify_one(); // powiadom fryzjera
        }
        else {
            // brak miejsca w poczekalni
            cout << "Klient " << id << " opuszcza salon z powodu braku miejsca." << endl;
            lockPoczekalnia.unlock();
            continue;
        }
        lockPoczekalnia.unlock();

        // czeka na zakonczenie uslugi i otrzymanie reszty (symulacja)

        if (sygnal2) {
            cout << "Klient " << id << " opuszcza salon ze wzgledu na sygnal 2." << endl;
            break;
        }

        // opuszcza salon i wraca do zarabiania pieniedzy


        // sprawdzenie czy salon nie zostal zamkniety podczas czekania
        if (!salonOtwarty) {
            cout << "Klient " << id << " opuszcza salon." << endl;
            break;
        }
    }
}