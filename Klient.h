#pragma once

#include <thread>

using namespace std;

class Klient {
public:
    int id;
    std::thread th;

    Klient(int id);

    void start();
    void dzialaj() const;
};