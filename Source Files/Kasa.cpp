#include "Kasa.h"

Kasa::Kasa(int b10, int b20, int b50)
	: banknoty10(b10), banknoty20(b20), banknoty50(b50) { }

void Kasa::dodajBanknot(int nominal) {

    lock_guard<mutex> lock(mtx); // blokada muteksa, break - zwolnienie

    switch (nominal) 
    {
    case 10:
        banknoty10++;
        break;
    case 20:
        banknoty20++;
        break;
    case 50:
        banknoty50++;
        break;
    default:
        break;
    }
    cv.notify_all(); // powiadom fryzjerow oczekujacych na reszte
}

bool Kasa::wydajReszte(int reszta, int& wydane10, int& wydane20, int& wydane50)
{
    unique_lock<mutex> lock(mtx); // blokada muteksa

    while (true) 
    {
        int temp10 = banknoty10;
        int temp20 = banknoty20;
        int temp50 = banknoty50;

        int r = reszta;
        wydane10 = wydane20 = wydane50 = 0;

        // algorytm wydawania reszty
        while (r >= 50 && temp50 > 0) {
            r -= 50;
            temp50--;
            wydane50++;
        }
        while (r >= 20 && temp20 > 0) {
            r -= 20;
            temp20--;
            wydane20++;
        }
        while (r >= 10 && temp10 > 0) {
            r -= 10;
            temp10--;
            wydane10++;
        }

        if (r == 0) {
            // wydaj reszte
            banknoty10 = temp10;
            banknoty20 = temp20;
            banknoty50 = temp50;
            return true;
        }
        else {
            // nie wydawaj reszty, czekaj na wplate nowych banknotow
            cv.wait(lock);
        }
    }
}