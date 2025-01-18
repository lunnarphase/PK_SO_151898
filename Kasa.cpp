
// Kasa.cpp

#include "Kasa.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>

Kasa::Kasa() {
    banknoty10 = nullptr;
    banknoty20 = nullptr;
    banknoty50 = nullptr;
}

Kasa::~Kasa() {}

void Kasa::initSharedMemory() {
    shmid = shmget(SHMKEY_KASA, 3 * sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Blad: shmget");
        exit(EXIT_FAILURE);
    }

    int* shared_mem = (int*)shmat(shmid, nullptr, 0);
    if (shared_mem == (void*)-1) {
        perror("Blad: shmat");
        exit(EXIT_FAILURE);
    }

    banknoty10 = &shared_mem[0];
    banknoty20 = &shared_mem[1];
    banknoty50 = &shared_mem[2];

    // Inicjalizuj banknoty
    *banknoty10 = 10;
    *banknoty20 = 10;
    *banknoty50 = 5;
}

void Kasa::removeSharedMemory() {
    if (shmdt(banknoty10) == -1) {
        perror("Blad: shmdt");
    }

    if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
        perror("Blad: shmctl");
    }
}

void Kasa::initSemaphore() {
    semid = semget(SEMKEY_KASA, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("Blad: semget");
        exit(EXIT_FAILURE);
    }

    // Inicjalizuj semafor na 1
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("Blad: semctl");
        exit(EXIT_FAILURE);
    }
}

void Kasa::removeSemaphore() {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("Blad: semctl IPC_RMID");
    }
}

void Kasa::dodajBanknot(int nominal) {
    // Semaphore wait (P operation)
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;  // Czekaj
    sem_op.sem_flg = 0;

    if (semop(semid, &sem_op, 1) == -1) {
        perror("Blad: semop wait");
        exit(EXIT_FAILURE);
    }

    switch (nominal) {
    case 10:
        (*banknoty10)++;
        break;
    case 20:
        (*banknoty20)++;
        break;
    case 50:
        (*banknoty50)++;
        break;
    default:
        break;
    }

    // Semafor (operacja V)
    sem_op.sem_op = 1;  // Sygnal
    if (semop(semid, &sem_op, 1) == -1) {
        perror("Blad: Wystapil blad uzycia sygnalu semop");
        exit(EXIT_FAILURE);
    }
}

bool Kasa::wydajReszte(int reszta, int& wydane10, int& wydane20, int& wydane50) {
    // Semafor 'czekaj' (operacja P)
    struct sembuf sem_op;
    sem_op.sem_num = 0;
    sem_op.sem_op = -1;  // Czekaj
    sem_op.sem_flg = 0;

    if (semop(semid, &sem_op, 1) == -1) {
        perror("Blad: semop wait");
        exit(EXIT_FAILURE);
    }

    // Logika sluzaca do obliczenia reszty
    int temp10 = *banknoty10;
    int temp20 = *banknoty20;
    int temp50 = *banknoty50;

    int r = reszta;
    wydane10 = wydane20 = wydane50 = 0;

    // Algorytm obliczajacy
    while (r >= 50 && temp50 > 0) {
        r -= 50;
        temp50--;
        wydane50++;
    }
    while (r >= 20 && temp20 > 0) {
        r -= 20;
        temp20--;
        wydane20++;
    }
    while (r >= 10 && temp10 > 0) {
        r -= 10;
        temp10--;
        wydane10++;
    }

    if (r == 0) {
        // Zaktualizuj pamiec wspoldzielona nowa iloscia banknotow
        *banknoty10 = temp10;
        *banknoty20 = temp20;
        *banknoty50 = temp50;

        // Semafor (operacja V)
        sem_op.sem_op = 1;  // Sygnal
        if (semop(semid, &sem_op, 1) == -1) {
            perror("Blad: Wystapil blad uzycia sygnalu semop");
            exit(EXIT_FAILURE);
        }
        return true;
    } else {
        // Nie mozna wydac reszty, klient musi zaczekac
        // Semafor (V operation)
        sem_op.sem_op = 1;  // Sygnal
        if (semop(semid, &sem_op, 1) == -1) {
            perror("Blad: Wystapil blad uzycia sygnalu semop");
            exit(EXIT_FAILURE);
        }
        return false;
    }
}
