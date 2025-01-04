#pragma once

#include <queue>
#include <mutex>

using namespace std;

class Salon {
public:
	int wolneFotele;
	int pojemnoscPoczekalni;

	queue<int> kolejkaKlientow; // kolejka klientow w poczekalni

	mutex mtxPoczekalnia; // muteks do synchronizacji poczekalni
	mutex mtxFotele; // muteks do synchronizacji dostepu do foteli

	condition_variable cvPoczekalnia; // zmienna warunkowa dla fryzjerow czekajacych na klientow

	Salon(int nFotele, int kPoczekalnia);
};