/**
 * @file ESP8266Client.h
 * @brief Client-facade to the ESP8266 library. Essentially ensures the
 * WLAN connection is set up prior to any IP connection attempts.
 * @author Christian Treber (ct@ctreber.com)
 * @date January 2017
 *
 * @par Copyright: GPL.
 */
#ifndef __ESP8266Client_H__
#define __ESP8266Client_H__

// Adjust this to the used Arduino HW serial and the ESP baud setting
#define ESP_SERIAL Serial1
#define ESP_BAUD 115200

#include "Client.h"
#include "ESP8266.h"
#include <Logsury.h>

#define WLAN_STATUS_UNKNOWN 255
#define WLAN_STATUS_CONNECTED 0

#define CONNECTION_STATUS_UNKNOWN 255
#define CONNECTION_STATUS_CONNECTED 0
#define CONNECTION_STATUS_DISCONNECTED 1
/**
 * Provide an easy-to-use way to manipulate ESP8266.
 */
class ESP8266Client : public Client {
 public:
    // From Print
    size_t write(uint8_t pData);
    size_t write(const uint8_t *pBuffer, size_t pSize);

    // From Stream (see https://www.arduino.cc/en/Reference/Stream)
    int available();
    int read();
    int peek();
    void flush();

    // From Client (see https://www.arduino.cc/en/Reference/ClientConstructor)
    int connect(IPAddress pIPaddress, uint16_t pPort);
    int connect(const char *pHost, uint16_t pPort);
    int read(uint8_t *pBuffer, size_t pSize);
    void stop();
    uint8_t connected();
    operator bool();

    // Self
    ESP8266Client(ESP8266 &pESP8266);
    /**
     * Implies tsetting up the WLAN chip (that is, calls setupServer()).
     */
    void setup(const char *pSSID, const char *pPassword);
    /**
     * Returns the client status (0: Ready for Client oprations).
     */
    int status();
    /**
     * Returns the local IP address of the client after a connection has been
     * made, or null if there is no connection.
     */
    IPAddress localIP();
    void setLogger(Logsury *pLogsury);

private:
    const char *_ssid;
    const char *_password;

    // Purely informative: The number of times the WLAN setup has been
    // executed.
    uint8_t _setupCount = 0;
    // The number of errors since the last WLAN setup. This could be
    // used as a hint that something is foobar, and the WLAN needs to
    // be set up again.
    uint8_t _errorCount = 0;

    // -1: Undetermined
    // 0: Connected to WLAN
    // 1: WLAN connect error
    uint8_t _wlanStatus = WLAN_STATUS_UNKNOWN;

    ESP8266 *_esp;
    Logsury *_logsury;

    /**
     * Sets up the WLAN component to be ready for Client operations. This
     * method does not block. Poll status() for information on readiness.
     */
    void setupWLAN();

    // -1: Undetermined
    // 0: Connected
    // 1: Connect error
    // 2: Disconnected
    uint8_t _connectionStatus = -1;

    uint8_t _bufPos = 0;
    uint8_t _bufLen = 0;
    uint8_t _buffer[100];

    void  fillBuffer();
};

#endif /* #ifndef __ESP8266Client_H__ */
