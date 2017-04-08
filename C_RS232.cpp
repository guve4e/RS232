
#include <fcntl.h>
#include <strings.h>
#include <cstring>
#include <unistd.h>
#include "C_RS232.h"

#define BAUDRATE B9600


#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

namespace RS232
{
    /*
     * Constructor
     * Opens a device and sets it up
     * @param deviceName - the name of the device to open
     */
    C_RS232::C_RS232(std::string deviceName) {

        // set device name
        this->deviceName = deviceName;

        // Open device for reading and writing and not as controlling tt
        // because we don't want to get killed if linenoise sends CTRL-C.

        m_fileDescriptor = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (m_fileDescriptor < 0) {
            // Log it here

            // make an exception class
            // with code and message
            char *s = strerror(errno);
            std::cout << s << " \n";

            throw -1;
        }

        std::cout<< deviceName << " opened\n";
        // General configuration
        //struct termios tty;
        struct termios config;
        memset(&config, 0, sizeof(config));
        tcgetattr(m_fileDescriptor, &config);
        config.c_iflag &= ~(INLCR | ICRNL);
        config.c_iflag |= IGNPAR | IGNBRK;
        config.c_oflag &= ~(OPOST | ONLCR | OCRNL);
        config.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
        config.c_cflag |= CLOCAL | CREAD | CS8;
        config.c_lflag &= ~(ICANON | ISIG | ECHO);
        //int flag = _BaudFlag(baudrate);
        cfsetospeed(&config, BAUDRATE);
        cfsetispeed(&config, BAUDRATE);
        // Timeouts configuration
        config.c_cc[VTIME] = 1;
        config.c_cc[VMIN] = 0;
        //fcntl(handle, F_SETFL, FNDELAY);
        // Validate configuration
        if (tcsetattr(m_fileDescriptor, TCSANOW, &config) < 0) {
            close(m_fileDescriptor);
            // Log it here

            // make an exception class
            // with code and message
            char *s = strerror(errno);
            std::cout << s << " \n";

            throw -1;
        }

        sleep(2); // make the arduino ready
    }

    /*
     * Reads data from device
     */
    void C_RS232::receive() {

    }

    /**
     * Robustly Write N bytes (unbuffered)
     */
    ssize_t C_RS232::send(int fd, void* usrbuf, size_t n)
    {
        size_t nleft = n;
        ssize_t nwritten;
        char* bufp = (char*)usrbuf;

        while (nleft > 0) {
            if ((nwritten = write(fd, bufp, nleft)) <= 0) {
                if (errno == EINTR)   // interrupted by sig handler return
                    nwritten = 0;     // and call write() again
                else
                    return -1;        // errno set by write()
            }
            nleft -= nwritten;
            bufp += nwritten;
        }
        return n;
    }

    /*
     * Sends string
     * @param data - string to send
     */
    void C_RS232::send(std::string data) {

        // convert std::string to char array
        char buffer[data.size()+1];
        strcpy(buffer, data.c_str());

        // get the size of the buffer
        ssize_t n = strlen(buffer);

        // send
        int res = send(m_fileDescriptor, buffer, n);
        if (res != n) {
             //make an exception class
            // with code and message
            char* s = strerror(errno);
            std::cout << s << " \n";
            throw -1;
        }
        std::cout << "Sent :" << res << " bytes\n";
    }

    /*
     * Closes the open port
     * Called from destructor
     */
    void C_RS232::closePort() {
        // restore the old port settings
        tcsetattr(m_fileDescriptor,TCSANOW,&m_oldTio);
        tcdrain(m_fileDescriptor);
        close(m_fileDescriptor);
        std::cout << this->deviceName << " closed\n";
    }

    /*
     * Destructor
     */
    C_RS232::~C_RS232() {
        closePort();
    }

    /*
     * Factory
     */
    I_RS232* R323Factory::R323(std::string device)
    {
        return new C_RS232(device);
    }
}






