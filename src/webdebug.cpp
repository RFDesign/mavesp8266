#include <WiFiUdp.h>

WiFiUDP debugserver;
IPAddress bip;
uint16_t debugport = 123;

void webdebug_init() {
    debugserver.begin(debugport);
    bip[0] = 192;
    bip[1] = 168;
    bip[2] = 4;
    bip[3] = 255;
}

void webdebug_publish(char * message, int len) {
    debugserver.beginPacket(bip, debugport);
    debugserver.write((const uint8_t *)message, len);
    debugserver.endPacket();
}