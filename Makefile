TARGETS=C_RS232.o main.o rs232

CC = g++
CFLAGS = -Wall -g -O0
GCC_VERSION = -std=c++1y 

all: $(TARGETS)

C_RS232.o: C_RS232.cpp C_RS232.h
	$(CC) $(CFLAGS) $(GCC_VERSION) -c C_RS232.cpp 
	
main.o: main.cpp
	$(CC) $(CFLAGS) $(GCC_VERSION) -c main.cpp 

rs232: main.o C_RS232.o
	$(CC) $(CFLAGS) $(GCC_VERSION) -o rs232 main.o C_RS232.o



clean:
	rm -f $(TARGETS)
