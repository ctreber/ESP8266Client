/**
 * @file ESP8266Client.cpp
 * @brief ESP8266-based Client.
 * @author Christian Treber (ct@ctreber.com)
 * @date January 2017
 *
 * @par Copyright: GPL.
 */
#include "ESP8266Client.h"

ESP8266Client::ESP8266Client(ESP8266 &pESP8266) {
    _esp = &pESP8266;
}

// From Client
int ESP8266Client::connect(IPAddress pIPaddress, uint16_t pPort) {
    logg(F("ESP8266Client::connect(IP address, port)\r\n"));

    char lBuf[16];
    char *p = lBuf;
    for (int i = 0; i < 4; i++) {
        uint8_t lNum = pIPaddress[i];
        int lDiv = 100;
        for (int j = 0; j < 3; j++) {
            if (lNum >= lDiv) {
                *p++ = '0' + lNum / lDiv;
                lNum %= lDiv;
            }
            lDiv /= 10;
        }
        if (i < 3) {
            *p++ = '.';
        }
    }
    *p = 0;

    connect(lBuf, pPort);
}

// From Client
int ESP8266Client::connect(const char *pHost, uint16_t pPort) {
    logg("ESP8266Client::connect(host, port)... ");

    logg(F("createTCP() "));
    if (_esp->createTCP(pHost, pPort)) {
        logg(F("OK"));
        _connectionStatus = CONNECTION_STATUS_CONNECTED;
    } else {
        logg(F("NOK"));
        _connectionStatus = CONNECTION_STATUS_UNKNOWN;
        _errorCount++;
    }

    logg(F("\r\n"));

    return connected();
}

// From Client
uint8_t ESP8266Client::connected() {
    //logg("ESP8266Client::connected()... ");
    uint8_t lConnected = _connectionStatus == CONNECTION_STATUS_CONNECTED;
    /*
    if(lConnected) {
        logg(F("YES"));
    } else {
        logg(("NO"));
    }
    logg(F("\r\n"));
    */
    return lConnected;
}

// From Client/Stream
int ESP8266Client::available() {
    //logg("ESP8266Client::available()... ");

    logg("ESP8266Client::available() ");
    logg(_esp->available());
    logg("\r\n");

    if(_esp->available()) {
        fillBuffer();
    }

    int lAvailable = _bufPos < _bufLen ? 1 : 0;
    if(lAvailable) {
        logg("at ");
        logg(_bufPos);
        logg(" of ");
        logg(_bufLen);
        logg("\r\n");
    }

    return lAvailable;
}

// From Client/Stream
int ESP8266Client::peek() {
    logg("ESP8266Client::peek()\r\n");

    return _buffer[_bufPos];
}

// From Client/Stream
int ESP8266Client::read() {
    logg("ESP8266Client::read()\r\n");

    if(_bufPos < _bufLen) {
        return _buffer[_bufPos++];
    }

    return -1;
}

// From Client
int ESP8266Client::read(uint8_t *pBuffer, size_t pSize) {
    logg(F("ESP8266Client::read(buffer, size) NOT IMPLEMENTED\r\n"));
}

// From Print
size_t ESP8266Client::write(uint8_t pData) {
    logg(F("ESP8266Client::write(data) NOT IMPLEMENTED\r\n"));
}

// From Print
size_t ESP8266Client::write(const uint8_t *pBuffer, size_t pSize) {
    logg(F("ESP8266Client::write(buffer, "));
    logg(pSize);
    logg(F(")... "));

    if(_esp->send(pBuffer, pSize)) {
        logg(F("OK"));
    } else {
        logg(F("NOK"));
        _errorCount++;
    }
    logg(F("\r\n"));

    return pSize;
}

// From Client/Stream
void ESP8266Client::flush() {
    logg(F("ESP8266Client::flush() NOT IMPLEMENTED\r\n"));
}

// From Client
void ESP8266Client::stop() {
    logg("ESP8266Client::stop()... ");

    logg(F("releaseTCP() "));
    if (_esp->releaseTCP()) {
        logg(F("OK"));
        _connectionStatus = CONNECTION_STATUS_DISCONNECTED;
    } else {
        logg(F("NOK"));
        _connectionStatus = CONNECTION_STATUS_UNKNOWN;
        _errorCount++;
    }
    logg(F("\r\n"));
}

// From Client
ESP8266Client::operator bool() {
    logg(F("ESP8266Client::operator bool() NOT IMPLEMENTED\r\n"));
}

void ESP8266Client::fillBuffer() {
    logg("ESP8266Client::fillBuffer()... ");

    _bufLen = _esp->recv(_buffer, sizeof(_buffer), 1000);
    _bufPos = 0;
    logg(_bufLen);
    logg(F(" bytes read\r\n"));
}

// Self
int ESP8266Client::status() {
    logg("ESP8266Client::status() NOT IMPLEMENTED\r\n");

    return _wlanStatus;
}

// Self
IPAddress ESP8266Client::localIP() {
    logg(F("ESP8266Client::localIP()\r\n"));

    IPAddress lIPAddress = IPAddress();
    lIPAddress.fromString(_esp->getLocalIP().c_str());
    return lIPAddress;
}

void ESP8266Client::setup(const char *pSSID, const char *pPassword) {
    logg("ESP8266Client::setup()\r\n");

    _ssid = pSSID;
    _password = pPassword;
    setupWLAN();
}

void ESP8266Client::setupWLAN() {
    logg(F("ESP8266Client::setupWLAN()... "));

    boolean lWLANConnected = false;

    while (_wlanStatus != WLAN_STATUS_CONNECTED) {
        logg(F("FW version: "));
        logg(_esp->getVersion().c_str());

        lWLANConnected = true;
        logg(F(" to station "));
        if (_esp->setOprToStation()) {
            logg(F("OK"));
        } else {
            logg(F("NOK"));
            lWLANConnected = false;
        }

        logg(F(" join AP "));
        if (_esp->joinAP(_ssid, _password)) {
            logg(F("OK IP: "));
            logg(_esp->getLocalIP().c_str());
        } else {
            logg(F("NOK"));
            // Once saw NOK while ping works just fine...
            lWLANConnected = false;
        }

        logg(F(" disable MUX "));
        if (_esp->disableMUX()) {
            logg(F("OK"));
        } else {
            logg(F("NOK"));
            lWLANConnected = false;
        }

        logg(F(" connection setup "));
        if (lWLANConnected) {
            logg(F("OK"));
            _wlanStatus = WLAN_STATUS_CONNECTED;
            _errorCount = 0;
        } else {
            logg(F("NOK"));
            _wlanStatus = WLAN_STATUS_UNKNOWN;
            _errorCount++;
        }

        _setupCount++;
        logg(F("\r\n"));
    }
}

void ESP8266Client::setLogger(Logsury *pLogsury) {
    _logsury = pLogsury;
}
