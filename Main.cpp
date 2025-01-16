// Main.cpp

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <limits>
#include <pthread.h>
#include <vector>
#include <sys/ipc.h>  // uzywane do ftok

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

std::vector<pid_t> clientPIDs; // przechowuje PID-y procesow klientów
std::vector<pid_t> barberPIDs; // przechowuje PID-y procesow fryzjerow

Salon salon(0, 0);  
Kasa kasa;          

int main()
{
    srand(time(nullptr));

    cout << "Uruchamianie programu ..." << endl; // Debug

    pobierzKonfiguracje();

    cout << "\nKonfiguracja ukonczona." << endl; // Debug

    FILE *fp;

    fp = fopen("salon_msgqueue", "w");
    if (fp == NULL) {
        perror("Blad: Nie udalo sie utworzyc pliku 'salon_msgqueue' dla ftok");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    fp = fopen("salon_shmkey_fotele", "w");
    if (fp == NULL) {
        perror("Blad: Nie udalo sie utworzyc pliku 'salon_shmkey_fotele' dla ftok");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    fp = fopen("kasa_shmkey", "w");
    if (fp == NULL) {
        perror("Blad: Nie udalo sie utworzyc pliku 'kasa_shmkey' dla ftok");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    // Inicjalizacja salon
    salon = Salon(N, K);

    kasa.initSharedMemory();
    kasa.initSemaphore();

    salon.initSharedMemory();
    salon.initSemaphores();

    signal(SIGUSR1, obslugaSygnalu1);  // Sygnal 1
    signal(SIGUSR2, obslugaSygnalu2);  // Sygnal 2

    // Tworzenie procesow fryzjerow
    for (int i = 1; i <= F; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Blad: Wystapil blad przy tworzeniu procesow fryzjerow");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Proces potomny - Fryzjer
            Fryzjer fryzjer(i, &salon, &kasa);
            fryzjer.dzialaj();
            exit(0);
        } else {
            // Proces rodzic - zapisz PID fryzjera
            barberPIDs.push_back(pid);
        }
    }

    cout << "\n## Fryzjerzy utworzeni ##" << endl;

    // Rozpocznij symulacje czasu w osobnym watku
    pthread_t czasThread;
    int ret = pthread_create(&czasThread, nullptr, symulujCzas, nullptr);
    if (ret != 0) {
        perror("Blad: Nie udalo sie utworzyc watku czasu !!!");
    }

    cout << "\n### Symulacja czasu uruchomiona ###" << endl;

    // Tworzenie procesow klientow
    for (int i = 1; i <= LICZBA_KLIENTOW; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Blad: Nie udalo sie utworzyc procesu klienta");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Proces potomny - Klient
            Klient klient(i, &salon, &kasa);
            klient.dzialaj();
            exit(0);
        } else {
            // Proces rodzic - zapisz PID klienta
            clientPIDs.push_back(pid);
        }
        sleep(rand() % 3 + 1);  // Losowy czas miedzy przyjsciem klientow
    }


    // Zaczekaj na wszystkie procesy potomne
    while (wait(NULL) > 0);

    // Dolacz watek czasu
    if (pthread_join(czasThread, nullptr) != 0) {
        perror("Blad: Nie udalo sie dolaczyc watku czasu !!!");
    }

    cout << "\n=#= Salon zakonczyl prace =#=" << endl;

    // Wyczysc wspoldzielone zasoby
    kasa.removeSharedMemory();
    kasa.removeSemaphore();
    salon.removeSharedMemory();
    salon.removeSemaphores();

    return 0;
}

void pobierzKonfiguracje() 
{
    cout << "Wprowadz liczbe fryzjerow (F > 1): ";
    while (!(cin >> F) || F <= 1) {
        cout << "Niepoprawna wartosc. Liczba fryzjerow musi byc wieksza niz 1. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Include <limits>
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

    cout << "Wprowadz godzine zamkniecia salonu (0 - 24, wieksza od godziny otwarcia): ";
    while (!(cin >> Tk) || Tk <= Tp || Tk > 24) {
        cout << "Niepoprawna wartosc. Godzina zamkniecia musi byc wieksza od godziny otwarcia i w przedziale 1 - 24. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void* symulujCzas(void* arg) {
    aktualnaGodzina = Tp;

    while (aktualnaGodzina < Tk) 
    {
        cout << "\n|| Aktualna godzina w salonie: " << aktualnaGodzina << ":00 ||\n" << endl;
        sleep(5);   // 5 sekund odpowiada 1 godzinie w salonie

        aktualnaGodzina++;

        if (aktualnaGodzina >= Tk) {
            cout << "\n!!! Salon wlasnie zostal zamkniety !!!\n" << endl;
            salonOtwarty = false;   // Flaga informujaca, ze salon jest zamkniety

            // Wyslij sygnał 2 do wszystkich procesow klientow
            for (pid_t pid : clientPIDs) {
                kill(pid, SIGUSR2);
            }

            for (pid_t pid : barberPIDs) {
                kill(pid, SIGUSR2);
            }

            break;
        }
    }
    return nullptr;
}
