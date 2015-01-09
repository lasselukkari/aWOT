#ifndef StreamClient_h
#define StreamClient_h

#include <Stream.h>
#include <Client.h>

class StreamClient : public Client
{
public:

    typedef void Stop();
    typedef int HostConnect(const char* host, uint16_t port);
    typedef int IPConnect(IPAddress ip, uint16_t port);

    StreamClient(Stream& stream) : m_stream(&stream), m_stop(NULL), m_hostConnect(NULL), m_ipConnect(NULL) {}

    // Prepare the client
    bool begin();

    // Connect
    int connect(const char* host, uint16_t port);
    int connect(IPAddress ip, uint16_t port);

    // Available
    int available();

    // Read
    int read();
    int read(uint8_t* buffer, size_t size);

    // Write
    size_t write(uint8_t b);
    size_t write(const char* data);
    size_t write(const uint8_t* buffer, size_t size);

    // Peek
    int peek();

    // Flush
    void flush();

    // Stop
    void stop();

    // Connected
    uint8_t connected();

    // Connected
    operator bool();

    // Set connect callback used with hostname
    void onHostConnect(HostConnect* hostConnect);
    // Set connect callback used with IP-address
    void onIPConnect(IPConnect* ipConnect);
    // Set stop callback
    void onStop(Stop* stop);


protected:
    Stream* m_stream;
    HostConnect* m_hostConnect;
    IPConnect* m_ipConnect;
    Stop* m_stop;
};

#endif