#include <iostream>
#include <string>
#include <termios.h>

namespace RS232
{
    class I_RS232
    {
    public:
        virtual void send(std::string) = 0;
        virtual void receive() = 0;
        virtual ~I_RS232() = default;
    };

    class C_RS232 : public I_RS232
    {
    public:
        C_RS232(std::string name);
        ~C_RS232();
        void send(std::string) override;
        void receive() override;
        void closePort();

    private: // member functions
        ssize_t send(int, void*, size_t);

    private:
        int m_fileDescriptor;
        std::string deviceName;
        struct termios m_oldTio;
        struct termios m_newTio;
    };

    class R323Factory
    {
    public:
        I_RS232* R323(std::string);
    };
}



