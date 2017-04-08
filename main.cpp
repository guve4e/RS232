#include <iostream>
#include <cstring>
#include "C_RS232.h"


int main(int argc, char* argv[]) {

    try {

        RS232::R323Factory* factory = new RS232::R323Factory();

        RS232::I_RS232* port = factory->R323("/dev/ttyUSB0");

        port->send("A");

        delete factory;
        delete port;
    }
    catch (int x)
    {
        std::cout << "Exception \n";
    }

    return 0;
}