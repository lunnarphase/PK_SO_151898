#include "ObslugaSygnalu.h"
#include <iostream>

using namespace std;

volatile sig_atomic_t sygnal1 = 0;
volatile sig_atomic_t sygnal2 = 0;

void obslugaSygnalu1(int signum) {
    sygnal1 = 1;
    cout << "\nOdebrano sygnal 1. Fryzjer konczy prace." << endl;
}

void obslugaSygnalu2(int signum) {
    sygnal2 = 1;
    cout << "\nOdebrano sygnal 2. Wszyscy klienci opuszczaja salon." << endl;
}
