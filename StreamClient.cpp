#include "Arduino.h"
#include "StreamClient.h"

bool StreamClient::begin()
{
    return true;
}

int StreamClient::connect(const char* host, uint16_t port)
{
    if (m_hostConnect != NULL)
        return m_hostConnect(host, port);

    return 0;
}

int StreamClient::connect(IPAddress ip, uint16_t port)
{
    if (m_ipConnect != NULL)
        return m_ipConnect(ip, port);

    return 0;
}

int StreamClient::available()
{
    return m_stream->available();
}


int StreamClient::read()
{
    return m_stream->read();
}

int StreamClient::read(uint8_t* buffer, size_t size)
{

    if(m_stream->available()) {

        int read = 0;

        unsigned long timeOut = millis() + 5;

        while((m_stream->available() || timeOut > millis()) && read < size) {
            char c = m_stream->read();

            if (c != -1) {
                buffer[read++] = c;
                timeOut = millis() + 5;
            }
        }

        return read;

    }

    return -1;

}

size_t StreamClient::write(uint8_t b)
{
    return m_stream->write(b);
}

size_t StreamClient::write(const char* data)
{
    return m_stream->write(data);
}

size_t StreamClient::write(const uint8_t* buffer, size_t size)
{
    return m_stream->write(buffer, size);
}

int StreamClient::peek()
{
    return m_stream->peek();
}

void StreamClient::flush()
{
    return m_stream->flush();
}

void StreamClient::stop()
{
    if (m_stop != NULL)
        m_stop();
}

uint8_t StreamClient::connected()
{
    long now = millis();

    while (now + 10 > millis()) {
        if (m_stream->available())
            return true;
    }

    return false;
}

StreamClient::operator bool()
{
    return true;
}

void StreamClient::onHostConnect(HostConnect* hostConnect)
{
    m_hostConnect = hostConnect;
}

void StreamClient::onIPConnect(IPConnect* ipConnect)
{
    m_ipConnect = ipConnect;
}

void StreamClient::onStop(Stop* stop)
{
    m_stop = stop;
}
