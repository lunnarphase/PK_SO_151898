#include <iostream>
#include <thread>
#include <ctime>
#include <csignal>

#include "Salon.h"
#include "Kasa.h"
#include "Fryzjer.h"
#include "Klient.h"
#include "ObslugaSygnalu.h"

using namespace std;

void pobierzKonfiguracje();
void symulujCzas();

int F;               // liczba fryzjerow (F > 1)
int N;               // liczba foteli (N < F)
int K;               // pojemnosc poczekalni
int LICZBA_KLIENTOW; // poczatkowa liczba klientow

int Tp;                          // godzina otwarcia salonu
int Tk;                          // godzina zamkniecia salonu
int aktualnaGodzina;             // symulowana aktualna godzina
atomic<bool> salonOtwarty(true); // flaga informujaca o tym, czy salon jest otwarty

Salon salon(0, 0);    // poczatkowe wartosci, ktore zostana zainicjowane w funkcji pobierzKonfiguracje()
Kasa kasa(10, 10, 5);

int main() 
{
    srand(time(nullptr));

    pobierzKonfiguracje();

	salon = Salon(N, K);

    signal(SIGINT, obslugaSygnalu1);  // sygnal 1
    signal(SIGTERM, obslugaSygnalu2); // sygnal 2

	// ====================================================================================================
    // utworzenie fryzjerow

    vector<Fryzjer> fryzjerzy;
    for (int i = 1; i <= F; ++i) {
        fryzjerzy.emplace_back(i);
    }

    // uruchamianie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        f.start();
    }

    // ====================================================================================================

    thread watekCzasu(symulujCzas); // uruchomienie symulacji czasu w osobnym watku

    // ====================================================================================================
    // utworzenie klientow

    vector<Klient> klienci;
    for (int i = 1; i <= LICZBA_KLIENTOW; ++i) {
        klienci.emplace_back(i);
    }

    // uruchamianie watkow klientow w losowych odstepach czasu
    for (auto& k : klienci) {
        k.start();
        this_thread::sleep_for(chrono::seconds(rand() % 3 + 1));
    }

    // dolaczanie watkow klientow
    for (auto& k : klienci) {
        if (k.th.joinable()) {
            k.th.join();
        }
    }

    // dolaczanie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        if (f.th.joinable()) {
            f.th.join();
        }
    }

    if (watekCzasu.joinable()) {
        watekCzasu.join();
    }

    cout << "\n=#= Salon zakonczyl prace =#=" << endl;

    return 0;
}

void pobierzKonfiguracje() 
{
    cout << "Wprowadz liczbe fryzjerow (F > 1): ";

    while (!(cin >> F) || F <= 1) {
        cout << "Niepoprawna wartosc. Liczba fryzjerow musi byc wieksza niz 1. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz liczbe foteli w salonie (N < F): ";
    while (!(cin >> N) || N >= F || N <= 0) {
        cout << "Niepoprawna wartosc. Liczba foteli musi byc mniejsza niz liczba fryzjerow i wieksza od 0. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz pojemnosc poczekalni dla klientow (K >= 0): ";
    while (!(cin >> K) || K < 0) {
        cout << "Niepoprawna wartosc. Pojemnosc poczekalni nie mo?e by? ujemna. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz poczatkowa liczbe klientow (>= 0): ";
    while (!(cin >> LICZBA_KLIENTOW) || LICZBA_KLIENTOW < 0) {
        cout << "Niepoprawna wartosc. Liczba klientow nie moze byc ujemna. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz godzine otwarcia salonu (0 - 23): ";
    while (!(cin >> Tp) || Tp < 0 || Tp > 23) {
        cout << "Niepoprawna wartosc. Godzina otwarcia musi byc w przedziale 0 - 23. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz godzine zamkniecia salonu (0 - 23, wieksza od godziny otwarcia): ";
    while (!(cin >> Tk) || Tk <= Tp || Tk > 24) {
        cout << "Niepoprawna wartosc. Godzina zamkniecia musi by? wieksza od godziny otwarcia i w przedziale 1 - 24. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void symulujCzas() {
    aktualnaGodzina = Tp;

    while (aktualnaGodzina < Tk) 
    {
        cout << "\n|| Aktualna godzina w salonie: " << aktualnaGodzina << ":00 ||\n" << endl;
        this_thread::sleep_for(chrono::seconds(5));   // 5 sekund dzialania programu odpowiada 1 godzinie pracy salonu

        aktualnaGodzina++;

        if (aktualnaGodzina >= Tk) {
            cout << "\n!!! Salon wlasnie zostal zamkniety !!!\n" << endl;
            salonOtwarty = false;   // ustawiamy flage, informujaca o tym ze salon jest zamkniety
            break;
        }
    }
}
