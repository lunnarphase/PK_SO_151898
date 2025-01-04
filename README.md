# Salon Fryzjerski - Temat nr 17

## Opis zadania

Zadanie stanowi rozszerzenie problemu spiacego fryzjera/golibrody na wielu fryzjerow. Wiecej na temat problemu mozna przeczytac tutaj:  
[Problem spiacego golibrody - Wikipedia](https://pl.wikipedia.org/wiki/Problem_śpiącego_golibrody)

### Warunki poczatkowe:
- W salonie pracuje **F fryzjerow** (**F > 1**) i znajduje sie w nim **N foteli** (**N < F**).
- Salon jest czynny w godzinach od **Tp** do **Tk**.
- Klienci przychodza do salonu w losowych momentach czasu.
- W salonie znajduje sie **poczekalnia**, ktora moze pomiescic **K klientow** jednoczesnie.
- Kazdy klient rozlicza usluge z fryzjerem, przekazujac mu kwote za usluge przed rozpoczeciem strzyzenia.
- Fryzjer wydaje reszte po zakonczeniu obslugi klienta. W przypadku braku mozliwosci wydania reszty, klient musi zaczekac, az fryzjer znajdzie reszte w kasie.
- **Kasa jest wspolna dla wszystkich fryzjerow.**  
  Platnosc moze byc dokonana banknotami o nominalach **10 zl**, **20 zl** i **50 zl**.

---

## Zasada dzialania fryzjera (cyklicznie):
1. Wybiera klienta z poczekalni (ewentualnie czeka, jesli go jeszcze nie ma).
2. Znajduje wolny fotel.
3. Pobiera oplate za usluge i umieszcza ja we wspolnej kasie (oplata moze byc rowniez przekazana bezposrednio przez klienta, ale fryzjer musi znac kwote, aby wyliczyc reszte do wydania).
4. Realizuje usluge.
5. Zwalnia fotel.
6. Wylicza reszte i pobiera ja ze wspolnej kasy. Jesli nie jest to mozliwe, czeka, az pojawia sie odpowiednie nominaly w wyniku pracy innego fryzjera.
7. Przekazuje reszte klientowi.

---

## Zasada dzialania klienta (cyklicznie):
1. Zarabia pieniadze.
2. Przychodzi do salonu fryzjerskiego.
3. Jesli jest wolne miejsce w poczekalni:
   - Siada i czeka na obsluge (ewentualnie budzi fryzjera).
   - W przypadku braku miejsc opuszcza salon i wraca do zarabiania pieniedzy.
4. Po znalezieniu fryzjera placi za usluge.
5. Czeka na zakonczenie uslugi.
6. Czeka na reszte.
7. Opuszcza salon i wraca do zarabiania pieniedzy.

---

## Dodatkowe polecenia kierownika:
1. Na polecenie kierownika (**sygnal 1**) dany fryzjer konczy prace przed zamknieciem salonu.
2. Na polecenie kierownika (**sygnal 2**) wszyscy klienci (zarowno ci siedzacy na fotelach, jak i w poczekalni) natychmiast opuszczaja salon.

---

## Do zaimplementowania
Nalezy napisac procedury symulujace dzialanie salonu fryzjerskiego:
- **Kasjer (Kierownik)**
- **Fryzjer**
- **Klient**
