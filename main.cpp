#include <iostream>
#include <cstring>
#include "C_RS232.h"


int main(int argc, char* argv[]) {

    try
    {

        std::unique_ptr<RS232::RS232Factory> factory = std::make_unique<RS232::RS232Factory>();

        std::unique_ptr<RS232::I_RS232> port = factory->RS232("/dev/ttyUSB0");

        port->send("S");
        port->receive();
    }
    catch(RS232::RS232Exception& e)
    {
            e.getMessage();
    }
    catch(std::bad_alloc& e)
    {
        std::cerr << e.what();
    }

    return 0;
}