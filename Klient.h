#pragma once

#include <pthread.h>
#include <iostream>
#include <cerrno>

using namespace std;

class Klient {
public:
    int id;
    pthread_t th;

    Klient(int id);

    void start();
    static void* dzialaj(void* arg);
};
