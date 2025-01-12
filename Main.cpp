#include <iostream>
#include <thread>
#include <ctime>
#include <csignal>

#include "Salon.h"
#include "Kasa.h"
#include "Fryzjer.h"
#include "Klient.h"
#include "ObslugaSygnalu.h"
#include "Constants.h"

using namespace std;

void pobierzKonfiguracje();

int F; // liczba fryzjerow (F > 1)
int N; // liczba foteli (N < F)
int K; // pojemnosc poczekalni
int LICZBA_KLIENTOW; // poczatkowa liczba klientow

Salon salon(0, 0); // poczatkowe wartosci, ktore zostana zainicjowane w funkcji pobierzKonfiguracje()
Kasa kasa(10, 10, 5);

int main() {
    srand(time(nullptr));

    pobierzKonfiguracje();

	salon = Salon(N, K);

    signal(SIGINT, obslugaSygnalu1); // sygnal 1
    signal(SIGTERM, obslugaSygnalu2); // sygnal 2

    // utworzenie fryzjerow
    vector<Fryzjer> fryzjerzy;
    for (int i = 1; i <= F; ++i) {
        fryzjerzy.emplace_back(i);
    }

    // oruchamianie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        f.start();
    }

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

    cout << "Salon zakonczyl prace." << endl;

    return 0;
}

void pobierzKonfiguracje() 
{
    cout << "Wprowadz liczbe fryzjerow (F > 1): ";

    while (!(cin >> F) || F <= 1) {
        cout << "Niepoprawna wartosc. Liczba fryzjerow musi by? wieksza ni? 1. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz liczbe foteli w salonie (N < F): ";
    while (!(cin >> N) || N >= F || N <= 0) {
        cout << "Niepoprawna wartosc. Liczba foteli musi by? mniejsza ni? liczba fryzjerow i wieksza od 0. Spróbuj ponownie: ";
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
}
