#pragma once

#include <thread>
#include <iostream>

using namespace std;

class Fryzjer {
public:
    int id;
    std::thread th;

    Fryzjer(int id);

    void start();
    void dzialaj();
};