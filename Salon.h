#pragma once

#include <queue>
#include <pthread.h>

using namespace std;

class Salon {
public:
    int wolneFotele;
    int pojemnoscPoczekalni;

    queue<int> kolejkaKlientow; // kolejka klientow w poczekalni

    pthread_mutex_t mtxPoczekalnia; // muteks do synchronizacji poczekalni
    pthread_mutex_t mtxFotele;      // muteks do synchronizacji dostepu do foteli

    pthread_cond_t cvPoczekalnia;   // zmienna warunkowa dla fryzjerow czekajacych na klientow

    Salon(int nFotele, int kPoczekalnia);
    ~Salon(); // destruktor do zwalniania zasobow

    Salon& operator=(const Salon& other);
};
