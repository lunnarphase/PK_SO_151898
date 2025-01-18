// #include <cstdlib>
// #include <sys/ipc.h>
// #include <sys/types.h>
// #include <unistd.h>

#include <iostream>
#include <csignal>
#include <cerrno>
#include <ctime>
#include <limits>
#include <vector>
#include <sys/wait.h>

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

    cout << "\n## Uruchamianie programu ... ##\n" << endl; // Debug
    sleep(2);

    pobierzKonfiguracje();

    cout << "\n## Konfiguracja ukonczona pomyslnie ##" << endl; // Debug
    sleep(2);

    salon = Salon(N, K); // Inicjalizacja salon

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
        if (pid == 0) { // Proces potomny - Fryzjer
            Fryzjer fryzjer(i, &salon, &kasa);
            fryzjer.dzialaj();
            exit(0);
        } else { 
            barberPIDs.push_back(pid); // Proces rodzic - zapisz PID fryzjera
        }
    }

    cout << "\n## Fryzjerzy utworzeni ##" << endl;
    sleep(2);

    cout << "\n#==============================================#" << endl;
    cout << "\tSalon fryzjerski zostal otwarty. \n\tGodziny otwarcia: " << Tp << ":00 - " << Tk << ":00";
    cout << "\n#==============================================#" << endl;
    sleep(2);

    // Rozpoczecie symulacji czasu w osobnym watku
    pthread_t czasThread;
    int ret = pthread_create(&czasThread, nullptr, symulujCzas, nullptr);
    if (ret != 0) {
        perror("Blad: Nie udalo sie utworzyc watku czasu !!!");
    }

    sleep(2);

    // Tworzenie procesow klientow
    for (int i = 1; i <= LICZBA_KLIENTOW; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Blad: Nie udalo sie utworzyc procesu klienta");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) { // Proces potomny - Klient
            Klient klient(i, &salon, &kasa);
            klient.dzialaj();
            exit(0);
        } else {
            clientPIDs.push_back(pid); // Proces rodzic - zapisz PID klienta
        }
        sleep(rand() % 3 + 1);  // Losowy czas miedzy przyjsciem klientow
    }


    // Zaczekaj na wszystkie procesy potomne
    while (wait(NULL) > 0);

    // Dolacz watek czasu
    if (pthread_join(czasThread, nullptr) != 0) {
        perror("Blad: Nie udalo sie dolaczyc watku czasu !!!");
    }

    cout << "\n=#= Salon zakonczyl prace =#=\n" << endl;

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
        cout << "\nLiczba fryzjerow musi byc wieksza niz 1. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz liczbe foteli w salonie (N < F): ";
    while (!(cin >> N) || N >= F || N <= 0) {
        cout << "\nLiczba foteli musi byc mniejsza niż liczba fryzjerow i wieksza od 0. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz pojemnosc poczekalni dla klientow (K >= 0): ";
    while (!(cin >> K) || K < 0) {
        cout << "\nPojemnosc poczekalni nie moze byc ujemna. Sprobuj ponownie: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Wprowadz poczatkowa liczbe klientow (>= 0): ";
    while (!(cin >> LICZBA_KLIENTOW) || LICZBA_KLIENTOW < 0) {
        cout << "\nLiczba klientow nie moze byc ujemna. Sprobuj ponownie: ";
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

        cout << "\n|| Aktualna godzina w salonie: " << aktualnaGodzina << ":00 ||\n" << endl;
        sleep(1);

        sleep(10);   // odpowiada 1 godzinie w salonie
        aktualnaGodzina++;

        if (aktualnaGodzina >= Tk) {
            cout << "\n!!! Salon wlasnie zostal zamkniety !!!\n" << endl;
            salonOtwarty = false;
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
