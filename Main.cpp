#include <iostream>
#include <pthread.h>
#include <csignal>
#include <vector>
#include <unistd.h>
#include <limits>

#include "Salon.h"
#include "Kasa.h"
#include "Fryzjer.h"
#include "Klient.h"
#include "ObslugaSygnalu.h"

using namespace std;

void pobierzKonfiguracje();
void* symulujCzas(void* arg);

int F;               // liczba fryzjerow (F > 1)
int N;               // liczba foteli (N < F)
int K;               // pojemnosc poczekalni
int LICZBA_KLIENTOW; // poczatkowa liczba klientow

int Tp;                       // godzina otwarcia salonu
int Tk;                       // godzina zamkniecia salonu
int aktualnaGodzina;          // symulowana aktualna godzina
bool salonOtwarty = true;     // flaga informujaca o tym, czy salon jest otwarty

Salon salon(0, 0);            // inicjalizacja w funkcji pobierzKonfiguracje()
Kasa kasa(0, 0, 0);           // kasa zostanie zainicjowana w main()

int main() 
{
    srand(time(nullptr));

    cout << "Uruchamianie programu ..." << endl; // Debug

    pobierzKonfiguracje();

    cout << "\nKonfiguracja ukonczona." << endl; // Debug

    salon = Salon(N, K);
    kasa = Kasa(10, 10, 5); // inicjalizacja kasy

    signal(SIGINT, obslugaSygnalu1);  // sygnal 1
    signal(SIGTERM, obslugaSygnalu2); // sygnal 2

    // Utworzenie fryzjerow
    vector<Fryzjer> fryzjerzy;
    for (int i = 1; i <= F; ++i) {
        fryzjerzy.emplace_back(i);
    }

    cout << "\n## Fryzjerzy utworzeni ##" << endl;
    // Uruchamianie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        f.start();
    }

    cout << "\n## Watki fryzjerow uruchomione ##" << endl;

    // Uruchomienie symulacji czasu w osobnym watku
    pthread_t czasThread;
    int ret = pthread_create(&czasThread, nullptr, symulujCzas, nullptr);
    if (ret != 0) {
        perror("Blad: Nie udalo sie utworzyc watku czasu !!!");
    }

    cout << "\n### Symulacja czasu uruchomiona ###" << endl;

    // Utworzenie klientow
    vector<Klient> klienci;
    for (int i = 1; i <= LICZBA_KLIENTOW; ++i) {
        klienci.emplace_back(i);
    }

    // Uruchamianie watkow klientow w losowych odstepach czasu
    for (auto& k : klienci) {
        k.start();
        sleep(rand() % 3 + 1);
    }

    cout << "\n## Watki klientow uruchomione ##" << endl;

    // Dolaczanie watkow klientow
    for (auto& k : klienci) {
        if (pthread_join(k.th, nullptr) != 0) {
            perror("Blad: Nie udalo sie dolaczyc watku klienta !!!");
        }
    }

    cout << "\n## Dolaczono watki klientow ##" << endl;

    // Dolaczanie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        if (pthread_join(f.th, nullptr) != 0) {
            perror("Nie udalo sie dolaczyc watku fryzjera !!!");
        }
    }

    cout << "\n## Dolaczono watki fryzjerow ##" << endl;

    // Dolaczenie watku czasu
    if (pthread_join(czasThread, nullptr) != 0) {
        perror("Blad: Nie udalo sie dolaczyc watku czasu !!!");
    }

    cout << "\n=#= Salon zakonczyl prace =#=" << endl;

    return 0;
}

// Funkcja pobierajaca konfiguracje od użytkownika
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
        cout << "Niepoprawna wartosc. Liczba foteli musi byc mniejsza niż liczba fryzjerow i wieksza od 0. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz pojemnosc poczekalni dla klientow (K >= 0): ";
    while (!(cin >> K) || K < 0) {
        cout << "Niepoprawna wartosc. Pojemnosc poczekalni nie moze byc ujemna. Sprobuj ponownie: ";
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
        cout << "Niepoprawna wartosc. Godzina zamkniecia musi byc wieksza od godziny otwarcia i w przedziale 1 - 24. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Funkcja symulujaca czas dzialania salonu
void* symulujCzas(void* arg) {
    aktualnaGodzina = Tp;

    while (aktualnaGodzina < Tk) 
    {
        cout << "\n|| Aktualna godzina w salonie: " << aktualnaGodzina << ":00 ||\n" << endl;
        sleep(5);   // 5 sekund działania programu odpowiada 1 godzinie pracy salonu

        aktualnaGodzina++;

        if (aktualnaGodzina >= Tk) {
            cout << "\n!!! Salon wlasnie zostal zamkniety !!!\n" << endl;
            salonOtwarty = false;   // flaga informujaca, ze salon jest zamkniety
            break;
        }
    }
    return nullptr;
}
