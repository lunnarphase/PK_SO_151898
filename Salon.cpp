#include "Salon.h"

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

Salon::Salon(int nFotele, int kPoczekalnia)
    : pojemnoscPoczekalni(kPoczekalnia), liczbaFoteli(nFotele) {

    wolneFotele = new int;  // Alokacja pamieci dla liczby wolnych foteli
    *wolneFotele = nFotele; // Inicjalizacja liczby wolnych foteli

    // Inicjalizacja muteksów i zmiennych warunkowych
    if (pthread_mutex_init(&mtxPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji mtxPoczekalnia");
    }
    if (pthread_mutex_init(&mtxFotele, nullptr) != 0) {
        perror("Blad inicjalizacji mtxFotele");
    }
    if (pthread_cond_init(&cvPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji cvPoczekalnia");
    }
}

Salon::~Salon() { // Destruktor - usuwamy alokowaną pamięć i niszczymy muteksy
    
    delete wolneFotele; 
    pthread_mutex_destroy(&mtxPoczekalnia);
    pthread_mutex_destroy(&mtxFotele);
    pthread_cond_destroy(&cvPoczekalnia);
}

void Salon::initSharedMemory() 
{
    // Utworz segment pamieci wspoldzielonej dla foteli
    shmidFotele = shmget(SHMKEY_FOTELE, sizeof(int), 0600 | IPC_CREAT);
    if (shmidFotele == -1) {
        perror("Blad: shmget");
        exit(EXIT_FAILURE);
    }

    // Dolacz segment pamieci wspoldzielonej
    wolneFotele = (int*)shmat(shmidFotele, nullptr, 0);
    if (wolneFotele == (void*)-1) {
        perror("Blad: shmat");

        exit(EXIT_FAILURE);
    }

    *wolneFotele = liczbaFoteli; // Przypisz wartosc poczatkowa
}

void Salon::initSemaphores() 
{
    // Utworz semafor dla foteli
    semidFotele = semget(SEMKEY_FOTELE, 1, 0600 | IPC_CREAT);
    if (semidFotele == -1) {
        perror("Blad: semget for semidFotele");

        exit(EXIT_FAILURE);
    }

    // Ustaw wartosc semafora dla foteli
    if (semctl(semidFotele, 0, SETVAL, liczbaFoteli) == -1) {
        perror("Blad: semctl SETVAL for semidFotele");

        exit(EXIT_FAILURE);
    }

    // Utworz semafor dla poczekalni
    semidPoczekalnia = semget(SEMKEY_POCZEKALNIA, 1, 0600 | IPC_CREAT);
    if (semidPoczekalnia == -1) {
        perror("Blad: semget for semidPoczekalnia");

        exit(EXIT_FAILURE);
    }

    // Ustaw wartosc semafora dla poczekalni
    if (semctl(semidPoczekalnia, 0, SETVAL, pojemnoscPoczekalni) == -1) {
        perror("Blad: semctl SETVAL for semidPoczekalnia");

        exit(EXIT_FAILURE);
    }
}

void Salon::removeSharedMemory() 
{
    // Odlacz segment pamieci wspoldzielonej
    if (shmdt(wolneFotele) == -1) {
        perror("Blad: shmdt wolneFotele");
    }

    // Usun segment pamieci wspoldzielonej
    if (shmctl(shmidFotele, IPC_RMID, nullptr) == -1) {
        perror("Blad: shmctl shmidFotele");
    }
}

void Salon::removeSemaphores() 
{
    // Usun semafor dla foteli
    if (semctl(semidFotele, 0, IPC_RMID) == -1) {
        perror("Blad: semctl IPC_RMID semidFotele");
    }

    // Usun semafor dla poczekalni
    if (semctl(semidPoczekalnia, 0, IPC_RMID) == -1) {
        perror("Blad: semctl IPC_RMID semidPoczekalnia");
    }
}

Salon& Salon::operator=(const Salon& other) 
{
    if (this == &other) { return *this; }
    delete wolneFotele;

    // Przypisanie wartosci
    wolneFotele = new int;
    *wolneFotele = *other.wolneFotele;
    pojemnoscPoczekalni = other.pojemnoscPoczekalni;
    liczbaFoteli = other.liczbaFoteli;

    // Reinicjalizacja muteksów i zmiennych warunkowych
    pthread_mutex_destroy(&mtxPoczekalnia);
    pthread_mutex_destroy(&mtxFotele);
    pthread_cond_destroy(&cvPoczekalnia);

    // Inicjalizacja muteksów i zmiennych warunkowych
    if (pthread_mutex_init(&mtxPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji mtxPoczekalnia w operator=");
    }
    if (pthread_mutex_init(&mtxFotele, nullptr) != 0) {
        perror("Blad inicjalizacji mtxFotele w operator=");
    }
    if (pthread_cond_init(&cvPoczekalnia, nullptr) != 0) {
        perror("Blad inicjalizacji cvPoczekalnia w operator=");
    }

    return *this;
}
