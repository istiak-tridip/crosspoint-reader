#pragma once

#include <WebServer.h>

#include "GfxRenderer.h"

class ScreenCapture {
public:
    uint8_t* bufferX = nullptr;
    GfxRenderer* rendererPtr = nullptr;

    WebServer server{80};
    void start(GfxRenderer& renderer);
    void handleFrame(const uint8_t* buffer,size_t length);
};
