#include "Fryzjer.h"
#include "Salon.h"
#include "Kasa.h"
#include "Constants.h"

extern Salon salon;
extern Kasa kasa;
extern std::atomic<bool> sygnal1;

Fryzjer::Fryzjer(int id) : id(id) {}

void Fryzjer::start() { th = std::thread(&Fryzjer::dzialaj, this); }

void Fryzjer::dzialaj()
{
    while (true)
    {
        if (sygnal1) {
            // fryzjer konczy prac?
            cout << "Fryzjer " << id << " konczy prac?." << endl;
            break;
        }

        // pobierz klienta z poczekalni

        unique_lock<mutex> lockPoczekalnia(salon.mtxPoczekalnia);

        while (salon.kolejkaKlientow.empty()) {
            cout << "Fryzjer " << id << " czeka na klienta." << endl;
            salon.cvPoczekalnia.wait(lockPoczekalnia);
            if (sygnal1) break;
        }
        if (sygnal1) break;

        int klientId = salon.kolejkaKlientow.front();
        salon.kolejkaKlientow.pop();
        lockPoczekalnia.unlock();

        // znajdz wolny fotel
        unique_lock<mutex> lockFotele(salon.mtxFotele);

        if (salon.wolneFotele > 0) {
            salon.wolneFotele--;
            cout << "Fryzjer " << id << " obsluguje klienta " << klientId << "." << endl;
        }
        else {
            cout << "Brak wolnych foteli!" << endl;
            continue;
        }
        lockFotele.unlock();

        // symulacja pobierania op?aty i umieszczania w kasie
        int oplata = 50;
        int banknotKlienta = 50;
        kasa.dodajBanknot(banknotKlienta);

        // realizacja uslugi
        this_thread::sleep_for(chrono::seconds(2));

        // zwolnienie fotela
        lockFotele.lock();
        salon.wolneFotele++;
        lockFotele.unlock();

        // wyliczenie reszty i pobieranie z kasy
        int reszta = banknotKlienta - oplata;
        int wydane10 = 0, wydane20 = 0, wydane50 = 0;

        if (reszta > 0) {
            bool resztaWydana = kasa.wydajReszte(reszta, wydane10, wydane20, wydane50);

            if (resztaWydana) {
                cout << "Fryzjer " << id << " wyda? reszt? klientowi " << klientId << "." << endl;
            }
            else {
                // Nie powinno si? zdarzy?, poniewa? funkcja wydajReszte czeka a? b?dzie mo?na wyda? reszt?
            }
        }

        // Przeka? reszt? klientowi (symulacja)
        // ...

        if (sygnal1) {
            cout << "Fryzjer " << id << " ko?czy prac? po obs?udze klienta." << endl;
            break;
        }
    }
}