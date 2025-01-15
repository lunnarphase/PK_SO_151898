#pragma once

#include <pthread.h>
#include <iostream>

using namespace std;

class Fryzjer {
public:
    int id;
    pthread_t th;

    Fryzjer(int id);

    void start();
    static void* dzialaj(void* arg);
};
