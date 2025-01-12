#pragma once

#include <thread>
#include <iostream>

using namespace std;

class Klient {
public:
    int id;
    std::thread th;

    Klient(int id);

    void start();
    void dzialaj() const;
};