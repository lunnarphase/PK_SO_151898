#pragma once

#include <signal.h>

void obslugaSygnalu1(int signum);
void obslugaSygnalu2(int signum);

extern volatile sig_atomic_t sygnal1; // Zmienna globalna informujaca o sygnale 1
extern volatile sig_atomic_t sygnal2; // Zmienna globalna informujaca o sygnale 2
