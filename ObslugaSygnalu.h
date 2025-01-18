#pragma once

#include <signal.h>

void obslugaSygnalu1(int signum);
void obslugaSygnalu2(int signum);

extern volatile sig_atomic_t sygnal1;
extern volatile sig_atomic_t sygnal2;
