#include "ScreenCapture.h"

#include <Wifi.h>
#include <WebServer.h>
#include <esp32/rom/crc.h>

void ScreenCapture::start(GfxRenderer& renderer)
{
    rendererPtr = &renderer;  // Store pointer

    WiFi.mode(WIFI_STA);
    WiFi.begin("Rushan's WiFi 2G", "rushanajize");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());

    server.on("/frame", HTTP_GET, [this]() {
        size_t length = rendererPtr->getBufferSize();
        // static uint8_t frameCopy[48000];
        // memcpy(frameCopy, rendererPtr->getFrameBuffer(), length);
        // handleFrame(bufferX, length);

        static uint8_t frameCopy[48000];
        noInterrupts();
        memcpy(frameCopy, bufferX, length);
        interrupts();
        handleFrame(frameCopy, length);
    });

    server.begin();
    Serial.println("Server started");
}

void ScreenCapture::handleFrame(const uint8_t* buffer, size_t length)
{
    WiFiClient client = server.client();


    // Calculate CRC32 of the buffer
    uint32_t crc = crc32_le(0, buffer, length);

    server.setContentLength(length);
    server.send(200, "application/octet-stream", "");
    server.sendHeader("X-Frame-CRC", String(crc, HEX));

    client.write(buffer, length);
    client.flush();
}