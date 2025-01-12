#include <iostream>
#include <thread>
#include <ctime>
#include <csignal>

#include "Salon.h"
#include "Kasa.h"
#include "Fryzjer.h"
#include "Klient.h"
#include "ObslugaSygnalu.h"
#include "Constants.h"

using namespace std;

Salon salon(N, K);
Kasa kasa(10, 10, 5);

int main() {
    srand(time(nullptr));

    signal(SIGINT, obslugaSygnalu1); // sygnal 1
    signal(SIGTERM, obslugaSygnalu2); // sygnal 2

    // utworzenie fryzjerow
    vector<Fryzjer> fryzjerzy;
    for (int i = 1; i <= F; ++i) {
        fryzjerzy.emplace_back(i);
    }

    // oruchamianie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        f.start();
    }

    // utworzenie klientow
    vector<Klient> klienci;
    for (int i = 1; i <= LICZBA_KLIENTOW; ++i) {
        klienci.emplace_back(i);
    }

    // uruchamianie watkow klientow w losowych odstepach czasu
    for (auto& k : klienci) {
        k.start();
        this_thread::sleep_for(chrono::seconds(rand() % 3 + 1));
    }

    // dolaczanie watkow klientow
    for (auto& k : klienci) {
        if (k.th.joinable()) {
            k.th.join();
        }
    }

    // dolaczanie watkow fryzjerow
    for (auto& f : fryzjerzy) {
        if (f.th.joinable()) {
            f.th.join();
        }
    }

    cout << "Salon zako?czy? prac?." << endl;

    return 0;
}