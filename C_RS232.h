#include <iostream>
#include <string>
#include <memory>
#include <termios.h>

#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))

namespace RS232
{
    class I_RS232
    {
    public:
        DLL_PUBLIC  virtual void send(std::string) = 0;
        DLL_PUBLIC virtual void receive() = 0;
        virtual ~I_RS232() = default;
    };

    DLL_LOCAL class C_RS232 : public I_RS232
    {
    public:
        C_RS232(std::string port);
        ~C_RS232();
        void send(std::string) override;
        void receive() override;
        void closePort();

    private: // member functions
        ssize_t send(int, void*, size_t);

    private: // member attributes
        int m_fileDescriptor;
        std::string deviceName;
        struct termios m_oldTio;
        struct termios m_newTio;
        int m_boudRate;
        
    };

    class RS232Factory
    {
    public:
        DLL_PUBLIC auto RS232(std::string)->std::unique_ptr<I_RS232>;
    };

    DLL_PUBLIC class RS232Exception : public std::runtime_error
    {
    private:
        std::string m_message;
        std::string m_errno;
    public:
        RS232Exception(std::string) noexcept ;
        void getErrno() noexcept ;
        void getMessage() const noexcept;
    };
}



