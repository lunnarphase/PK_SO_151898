#include "ObslugaSygnalu.h"
#include "Constants.h"

#include <atomic>

using namespace std;

atomic<bool> sygnal1(false);
atomic<bool> sygnal2(false);

void obslugaSygnalu1(int signum) {
	sygnal1 = true;
}

void obslugaSygnalu2(int signum) {
	sygnal2 = true;
}