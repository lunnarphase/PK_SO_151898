CXX = g++
CXXFLAGS = -Wall -Wextra -pthread -std=c++17

EXEC = salon
OBJS = Main.o Salon.o Kasa.o Fryzjer.o Klient.o ObslugaSygnalu.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

Main.o: Main.cpp
	$(CXX) $(CXXFLAGS) -c Main.cpp

Salon.o: Salon.cpp Salon.h
	$(CXX) $(CXXFLAGS) -c Salon.cpp

Kasa.o: Kasa.cpp Kasa.h
	$(CXX) $(CXXFLAGS) -c Kasa.cpp

Fryzjer.o: Fryzjer.cpp Fryzjer.h
	$(CXX) $(CXXFLAGS) -c Fryzjer.cpp

Klient.o: Klient.cpp Klient.h
	$(CXX) $(CXXFLAGS) -c Klient.cpp

ObslugaSygnalu.o: ObslugaSygnalu.cpp ObslugaSygnalu.h
	$(CXX) $(CXXFLAGS) -c ObslugaSygnalu.cpp

clean:
	rm -f $(EXEC) *.o