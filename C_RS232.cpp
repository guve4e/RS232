#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <map>
#include "C_RS232.h"

namespace RS232
{
    std::map<int, int> boudRates = {
            {0, B0},
            {50, B50},
            {75, B75},
            {110, B110},
            {134, B134},
            {150, B150},
            {200, B200},
            {300, B300},
            {600, B600},
            {1200, B1200},
            {1800, B1800},
            {2400, B2400},
            {4800, B4800},
            {9600, B9600},
            {19200, B19200},
            {3840, B38400},
            {57600, B57600},
            {115200,B115200},
            {230400, B230400}
    };

    /**
     * Constructor
     * Opens a device and sets it up
     * @param m_port - the name of the device to open
     */
    C_RS232::C_RS232(std::string port)
    {
        // set device name
        this->m_port = port;
        // set default boud rate
        this->m_boudRate = B9600;

        // open port and set
        // file descriptor
        (void)openPort();
        std::cout<< port << " opened\n";
        // set up
        (void)setUp();
    }

    /**
     * Set UP
     *
     * @throws RS232Exception
     */
    void C_RS232::setUp()
    {
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
        cfsetospeed(&config, this->m_boudRate);
        cfsetispeed(&config, this->m_boudRate);
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

    /**
     * Open port
     *
     * @throws RS232Exception
     */
    void C_RS232::openPort()
    {
        // open m_port
        m_fileDescriptor = open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (m_fileDescriptor < 0)
        {
            throw RS232Exception("open");
        }
    }

    /**
     * Robustly Write N bytes (unbuffered)
     * On Error sets errno
     *
     * @param fd - file descriptor
     * @param usrbuf - void pointer (pointer to anything)
     * @param n - the size of the thing that usrbuf points to
     * @return number of bytes transferred if OK, 0 on EOF, −1 on error
     */
    ssize_t C_RS232::send(int fd, void* usrbuf, size_t n)
    {
        size_t nleft = n;
        ssize_t nwritten;
        char *bufp = static_cast<char*>(usrbuf);

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

    /**
     * Receive (unbuffered)
     * On Error sets errno
     *
     * @param fd - file descriptor
     * @param usrbuf - void pointer (pointer to anything)
     * @param n - the size of the thing that usrbuf points t
     * @return number of bytes read if OK, 0 on EOF, −1 on error
     */
    ssize_t C_RS232::receive(int fd, void *usrbuf, size_t n)
    {
        size_t nleft = n;
        ssize_t nread;

        char *bufp = static_cast<char*>(usrbuf);

        while (nleft > 0) {
            if ((nread = read(fd, bufp, nleft)) < 0) {
                if (errno == EINTR) /* Interrupted by sig handler return */
                    nread = 0; /* and call read() again */
                else
                    return -1; /* errno set by read() */
            } else if (nread == 0)
                break; /* EOF */

            nleft -= nread;
            bufp += nread;
        }

        return (n - nleft); /* Return >= 0 */
    }

    /**
     * Reads data from device
     */
    void C_RS232::receive() {

        char * a = new char;

        sleep(5);

        ssize_t result = receive(this->m_fileDescriptor,a,1);
        if (result < 0) throw new RS232Exception("Receive ");

        std::cout << "Received :" << a  << "\n";
    }

    /**
     * Send uint8_t
     * @param data
     */
    void C_RS232::send(uint8_t data) {

        int n = sizeof(data);
        uint8_t* buffer = &data;

        // send
        ssize_t res = send(m_fileDescriptor, buffer, n);
        if (res != n) {
            throw new RS232Exception("Send int");
        }
        std::cout << "Sent :" << res << " bytes\n";
    }

    /**
     * Sends string
     * @param data - string to send
     */
    void C_RS232::send(std::string data) {

        // convert std::string to char array
        std::unique_ptr<char> buffer = stringToCharArray(data);

        // get the size of the buffer
        ssize_t n = strlen(buffer.get());

        // send
        ssize_t res = send(m_fileDescriptor, buffer.get(), n);
        if (res != n) {
            throw new RS232Exception("Send string");
        }
        std::cout << "Sent :" << res << " bytes\n";
    }

    /*
     * Closes the open m_port
     * Called from destructor
     */
    void C_RS232::closePort() {
        // restore the old m_port settings
        tcsetattr(m_fileDescriptor,TCSANOW,&m_oldTio);
        tcdrain(m_fileDescriptor);
        close(m_fileDescriptor);
        std::cout << this->m_port << " closed\n";
    }

    /*
     * Destructor
     */
    C_RS232::~C_RS232() {
        closePort();
    }

    /*
     * Set Baud Rate
     * Default B9600
     */
    void C_RS232::setBaudRate(int rate) {
        // check invariant
        auto search = boudRates.find(rate);
        if(search == boudRates.end()) {
            throw new RS232Exception(std::to_string(rate)
                                     + " is not valid Boud Rate");
        }

        // set baoud rate
        this->m_boudRate = boudRates.at(rate);
    }

    /**
     * Converts std::string to char array
     *
     * @param data
     * @return pointer to char array
     */
    auto C_RS232::stringToCharArray(std::string data) -> std::unique_ptr<char>
    {
        // allocate enough space for the array
        auto buffer = std::make_unique<char>(data.size() + 1);
        strcpy(buffer.get(), data.c_str());
        return buffer;
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






