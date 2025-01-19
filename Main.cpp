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
#include <sys/ipc.h>
#include <vector>

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

Salon salon(0, 0);  // Will be initialized after configuration
Kasa kasa;          // Kasa instance

std::vector<pid_t> clientPIDs;  // Wektor PID-ów klientów
std::vector<pid_t> barberPIDs;  // Wektor PID-ów fryzjerów

int main()
{
    srand(time(nullptr));
    
    cout << "\n\033[1;30mUruchamianie programu ...\033[0m\n" << endl; // Debug
    sleep(1);

    pobierzKonfiguracje();

    cout << "\n\033[1;30mKonfiguracja ukonczona.\033[0m\n" << endl; // Debug
    sleep(1);

    salon = Salon(N, K);

    // Inicjalizacja pamieci wspoldzielonej i semaforow
    kasa.initSharedMemory();
    kasa.initSemaphore();
    salon.initSharedMemory();
    salon.initSemaphores();

    signal(SIGUSR1, obslugaSygnalu1);  // Sygnal 1
    signal(SIGUSR2, obslugaSygnalu2);  // Sygnal 2

    // Utworzenie procesow fryzjerow
    for (int i = 1; i <= F; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Blad: Wystapil blad przy tworzeniu procesow fryzjerow");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) { // Proces potomny - Fryzjer
            Fryzjer fryzjer(i, &salon, &kasa);
            fryzjer.dzialaj();
            exit(0);
        } else {
            barberPIDs.push_back(pid); // Proces rodzic - zapisuje PID fryzjera
        }
    }

    cout << "\n\033[1;30m## Fryzjerzy utworzeni ##\033[0m" << endl;
    sleep(1);

    // Utworzenie watku symulujacego czas
    pthread_t czasThread;
    int ret = pthread_create(&czasThread, nullptr, symulujCzas, nullptr);
    if (ret != 0) {
        perror("Blad: Nie udalo sie utworzyc watku czasu !!!");
    }

    cout << "\n\033[1;30m## Symulacja czasu uruchomiona ##\033[0m\n" << endl;
    sleep(2);

    // Utworzenie procesow klientow
    for (int i = 1; i <= LICZBA_KLIENTOW; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error creating client process");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) { // Proces potomny - Klient
            Klient klient(i, &salon, &kasa);
            klient.dzialaj();
            exit(0);
        } else { 
            clientPIDs.push_back(pid); // Proces rodzic - zapisuje PID klienta
        }
        sleep(rand() % 3 + 1);  // Losowy czas miedzy utworzeniem kolejnych klientow (1-3 sekundy)
    }

    // Oczekiwanie na zakonczenie watku symulujacego czas
    if (pthread_join(czasThread, nullptr) != 0) {
        perror("Blad: Nie udalo sie dolaczyc watku czasu !!!");
    }

    // Salon jest zamknięty - wyślij sygnały do klientów i fryzjerów
    for (pid_t pid : clientPIDs) { 
        kill(pid, SIGUSR2); // Sygnal 2
    }

    for (pid_t pid : clientPIDs) { 
        waitpid(pid, NULL, 0); // Czekaj na zakonczenie procesow klientow
    }

    for (pid_t pid : barberPIDs) {
        kill(pid, SIGUSR1); // Sygnal 1
    }

    for (pid_t pid : barberPIDs) {
        waitpid(pid, NULL, 0); // Czekaj na zakonczenie procesow fryzjerow
    }

    cout << "\n\033[1;31m=#= Salon zakonczyl prace =#=\033[0m\n" << endl;

    // Usuniecie pamieci wspoldzielonej i semaforow
    kasa.removeSharedMemory();
    kasa.removeSemaphore();
    salon.removeSharedMemory();
    salon.removeSemaphores();

    return 0;
}

void pobierzKonfiguracje() 
{
    cout << "Wprowadz liczbe fryzjerow (1 < F < 10): ";
    while (!(cin >> F) || F <= 1 || F >= 10) {
        cout << "\nLiczba fryzjerow musi byc wieksza niz 1 i mniejsza niz 10. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz liczbe foteli w salonie (N < F): ";
    while (!(cin >> N) || N >= F || N <= 0) {
        cout << "\nLiczba foteli musi byc mniejsza niz liczba fryzjerow i wieksza od 0. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz pojemnosc poczekalni dla klientow (K >= 0): ";
    while (!(cin >> K) || K < 0) {
        cout << "\nPojemnosc poczekalni nie moze byc ujemna. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz poczatkowa liczbe klientow (0 < LICZBA_KLIENTOW < 10): ";
    while (!(cin >> LICZBA_KLIENTOW) || LICZBA_KLIENTOW <= 0 || LICZBA_KLIENTOW >= 10) {
        cout << "\nLiczba klientow musi byc wieksza od 0 i mniejsza niz 10. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz godzine otwarcia salonu (0 - 23): ";
    while (!(cin >> Tp) || Tp < 0 || Tp > 23) {
        cout << "\nGodzina otwarcia musi byc w przedziale 0 - 23. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz godzine zamkniecia salonu (0 - 24, wieksza od godziny otwarcia): ";
    while (!(cin >> Tk) || Tk <= Tp || Tk > 24) {
        cout << "\nGodzina zamkniecia musi byc wieksza od godziny otwarcia i w przedziale 1 - 24. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void* symulujCzas(void* arg) {
    aktualnaGodzina = Tp;

    while (aktualnaGodzina < Tk) 
    {
        cout << "\n\033[1;32m|| Aktualna godzina w salonie: " << aktualnaGodzina << ":00 ||\033[0m\n" << endl;
        sleep(1);

        sleep(10);   // Czas odpowiadajacy 1 godzinie w salonie
        aktualnaGodzina++;

        if (aktualnaGodzina >= Tk) 
        {
            salonOtwarty = false;
            cout << "\n\033[1;31m!!! Salon wlasnie zostal zamkniety !!!\033[0m\n" << endl;
            sleep(2);

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