#include "ScreenCapture.h"

#include <Wifi.h>
#include <WebServer.h>

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
        static uint8_t frameCopy[48000];
        memcpy(frameCopy, rendererPtr->getFrameBuffer(), length);
        handleFrame(frameCopy, length);
    });

    server.begin();
    Serial.println("Server started");
}

void ScreenCapture::handleFrame(const uint8_t* buffer, size_t length)
{
    WiFiClient client = server.client();

    server.setContentLength(length);
    server.send(200, "application/octet-stream", "");

    client.write(buffer, length);
    client.flush();
}