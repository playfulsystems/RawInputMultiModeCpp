#pragma once
// RawInputPlugin.h
// Main export header for Unity Raw Input plugin

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <cstdint>

// --- Exported Types (matches your C# struct layout) ---

const int MAX_DEVICES = 32;
const int MAX_BUTTONS = 8;

struct RawMouseState {
    int deviceId;
    int deltaX;
    int deltaY;
    int wheel;
    unsigned char buttons[MAX_BUTTONS];      // 1 = down, 0 = up
    unsigned char buttonsPressed[MAX_BUTTONS];  // 1 = down this frame
    unsigned char buttonsReleased[MAX_BUTTONS]; // 1 = up this frame
};

// --- Exported API ---

extern "C" {

    DLLEXPORT int PollRawMouseInput(RawMouseState* buffer, int bufferSize);
    DLLEXPORT void StartRawInputThread();
    DLLEXPORT void StopRawInputThread();
    DLLEXPORT void SetForwardToUnity(bool enable);

    DLLEXPORT uint32_t GetRawInputEventCount();
    DLLEXPORT const uint32_t* GetRawInputHeaderIndices();
    DLLEXPORT const uint32_t* GetRawInputDataIndices();
    DLLEXPORT const uint8_t* GetRawInputDataBuffer(int* size);
    DLLEXPORT void ClearRawInputEventBuffers();

}

