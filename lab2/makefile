all: lab2

lab2: Sender.o Receiver.o
	g++ Sender.o Receiver.o -g -o lab2.o
Sender.o: Sender.cpp Receiver.h Sender.h Event.h
	g++ -c Sender.cpp
Receiver.o: Receiver.cpp Receiver.h Event.h
	g++ -c Receiver.cpp
clean:
	rm Sender.o Receiver.o lab2.o
