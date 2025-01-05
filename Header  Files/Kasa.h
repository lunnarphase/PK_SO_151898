#pragma once

#include <mutex>

using namespace std;

class Kasa {
public:
	int banknoty10;
	int banknoty20;
	int banknoty50;

	mutex mtxKasa; // muteks do synchronizacji dostepu do kasy
	condition_variable cvReszta; // zmienna warunkowa dla oczekujacych na reszte

	Kasa(int b10, int b20, int b50);

	void dodajBanknot(int nominal);
	bool wydajReszte(int reszta, int& wydane10, int& wydane20, int& wydane50);
};