#include "Kasa.h"
#include <cstdio>
#include <cerrno>

Kasa::Kasa(int b10, int b20, int b50)
    : banknoty10(b10), banknoty20(b20), banknoty50(b50) {
    if (pthread_mutex_init(&mtxKasa, nullptr) != 0) {
        perror("Blad inicjalizacji mtxKasa");
    }
    if (pthread_cond_init(&cvReszta, nullptr) != 0) {
        perror("Blad inicjalizacji cvReszta");
    }
}

void Kasa::dodajBanknot(int nominal) {

    if (pthread_mutex_lock(&mtxKasa) != 0) {
        perror("Blad przy zablokowaniu mtxKasa");
        return;
    }

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
    pthread_cond_broadcast(&cvReszta); // powiadom fryzjerow oczekujacych na reszte

    if (pthread_mutex_unlock(&mtxKasa) != 0) {
        perror("Blad przy odblokowaniu mtxKasa");
    }
}

bool Kasa::wydajReszte(int reszta, int& wydane10, int& wydane20, int& wydane50)
{
    if (pthread_mutex_lock(&mtxKasa) != 0) {
        perror("Blad przy zablokowaniu mtxKasa");
        return false;
    }

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
            pthread_mutex_unlock(&mtxKasa);
            return true;
        }
        else {
            // nie wydawaj reszty, czekaj na wplate nowych banknotow
            pthread_cond_wait(&cvReszta, &mtxKasa);
        }
    }
}