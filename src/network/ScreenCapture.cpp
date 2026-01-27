#include "ScreenCapture.h"

#include <WebServer.h>
#include <Wifi.h>
#include <esp32/rom/crc.h>

void ScreenCapture::start(GfxRenderer& renderer) {
  rendererPtr = &renderer;  // Store pointer

  WiFi.mode(WIFI_STA);
  WiFi.begin("Rushan's WiFi 2G", "rushanajize");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/frame", HTTP_GET, [this]() {
    const int width = rendererPtr->getScreenWidth();
    const int height = rendererPtr->getScreenHeight();
    const size_t length = rendererPtr->getBufferSize();

    static uint8_t frameCopy[48000];
    noInterrupts();
    memcpy(frameCopy, frameBuffer, length);
    interrupts();
    handleFrame(frameCopy, length, height, width);
  });

  server.begin();
  Serial.printf("Frame Server started: %s\n", WiFi.localIP().toString().c_str());
}

void ScreenCapture::handleFrame(const uint8_t* buffer, size_t length, int height, int width) {
  WiFiClient client = server.client();
  uint32_t crc = crc32_le(0, buffer, length);

  server.setContentLength(length);
  server.sendHeader("X-Frame-Width", String(width));
  server.sendHeader("X-Frame-Height", String(height));
  server.sendHeader("X-Frame-Hash", String(crc, HEX));
  server.send(200, "application/octet-stream", "");

  client.write(buffer, length);
  client.flush();
}