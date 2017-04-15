
#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include "C_RS232.h"

#define BAUDRATE B9600


namespace RS232
{
    /*
     * Constructor
     * Opens a device and sets it up
     * @param deviceName - the name of the device to open
     */
    C_RS232::C_RS232(std::string port) {

        // set device name
        this->deviceName = port;

        // open port
        m_fileDescriptor = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (m_fileDescriptor < 0)
        {
            throw RS232Exception("open");
        }

        std::cout<< port << " opened\n";

        // conf
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

        // Validate configuration
        if (tcsetattr(m_fileDescriptor, TCSANOW, &config) < 0) {
            close(m_fileDescriptor);
            // Log it here
            throw new RS232Exception("Unable to set attributes");
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
    auto RS232Factory::RS232(std::string port)->std::unique_ptr<I_RS232>
    {
        return std::make_unique<C_RS232>(port);
    }

    /*
     * RS232Exception Constructor
     */
    RS232Exception::RS232Exception(std::string msg) noexcept
            : std::runtime_error( msg ), m_message(msg), m_errno("")
    {
        (void)getErrno();
    }

    /*
     * Gets Info from Environmental Variable
     * Errno and stores it in the m_errno member
     */
    void RS232Exception::getErrno() noexcept
    {
        // get value from errno
        char *s = strerror(errno);
        if (s != nullptr)
        {
            std::string str(s);
            this->m_errno = str;
        }
    }

    /*
     * String Representation of the Exception
     */
    void RS232Exception::getMessage() const noexcept
    {
        std::cerr << "Message: " <<  this->m_message << std::endl;
        std::cerr << "Errno : " << this->m_errno << std::endl;
        std::cerr << this->what();
    }
}






