#include "RawInputStateTracker.h"
#include <cstring>

// Called from window proc for every raw input event
void RawInputStateTracker::HandleRawInput(const RAWINPUT* raw) {
    if (!raw || raw->header.dwType != RIM_TYPEMOUSE)
        return;

    // enforce that only one thread at a time can run the protected code
    std::lock_guard<std::mutex> lock(stateMutex);

    HANDLE handle = raw->header.hDevice;
    // Assign unique device id if first time seeing device
    if (deviceIdMap.find(handle) == deviceIdMap.end())
        deviceIdMap[handle] = nextDeviceId++;

    RawMouseState& state = deviceStates[handle];
    state.deviceId = deviceIdMap[handle];
    state.deltaX += raw->data.mouse.lLastX;
    state.deltaY += raw->data.mouse.lLastY;
    state.wheel = 0;

    USHORT flags = raw->data.mouse.usButtonFlags;
    if (flags & RI_MOUSE_WHEEL)
        state.wheel = (SHORT)raw->data.mouse.usButtonData;

    // init prev buttons
    unsigned char prevButtons[MAX_BUTTONS] = { 0 };
    std::memcpy(prevButtons, state.buttons, sizeof(prevButtons));

    // Update button down state
    std::memset(state.buttons, 0, sizeof(state.buttons));
    std::memset(state.buttonsPressed, 0, sizeof(state.buttonsPressed));
    std::memset(state.buttonsReleased, 0, sizeof(state.buttonsReleased));

    if (flags & RI_MOUSE_LEFT_BUTTON_DOWN)   state.buttons[0] = 1;
    if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN)  state.buttons[1] = 1;
    if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) state.buttons[2] = 1;
    if (flags & RI_MOUSE_BUTTON_4_DOWN)      state.buttons[3] = 1;
    if (flags & RI_MOUSE_BUTTON_5_DOWN)      state.buttons[4] = 1;
    if (flags & RI_MOUSE_LEFT_BUTTON_UP)     state.buttons[0] = 0;
    if (flags & RI_MOUSE_RIGHT_BUTTON_UP)    state.buttons[1] = 0;
    if (flags & RI_MOUSE_MIDDLE_BUTTON_UP)   state.buttons[2] = 0;
    if (flags & RI_MOUSE_BUTTON_4_UP)        state.buttons[3] = 0;
    if (flags & RI_MOUSE_BUTTON_5_UP)        state.buttons[4] = 0;

    // Edge detection: just pressed/released this frame
    // If previous was 0 and now is 1, it's a press. If previous was 1 and now is 0, it's a release.
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        if (prevButtons[i] == 0 && state.buttons[i] == 1)
            state.buttonsPressed[i] = 1;
        if (prevButtons[i] == 1 && state.buttons[i] == 0)
            state.buttonsReleased[i] = 1;
    }
}

// Called from Unity, fills array with current state and resets deltas/buttons
int RawInputStateTracker::PollMouseStates(RawMouseState* buffer, int bufferSize) {

    // enforce that only one thread at a time can run the protected code
    std::lock_guard<std::mutex> lock(stateMutex);
    
    int index = 0;
    // Iterate through all tracked devices in the deviceStates map
    for (const auto& entry : deviceStates) {
        // If the buffer is full, stop adding more devices
        if (index >= bufferSize) break;

        // entry.first is the device HANDLE (the key)
        // entry.second is the RawMouseState struct for that device (the value)
        // Copy this device's state into the buffer at the current index
        buffer[index++] = entry.second;
    }

    // Reset per-frame state (deltas, wheel, buttons)
    for (auto& entry : deviceStates) {
        entry.second.deltaX = 0;
        entry.second.deltaY = 0;
        entry.second.wheel = 0;
        std::memset(entry.second.buttonsPressed, 0, sizeof(entry.second.buttonsPressed));
        std::memset(entry.second.buttonsReleased, 0, sizeof(entry.second.buttonsReleased));
    }
    return index;
}

void RawInputStateTracker::Reset() {

    // enforce that only one thread at a time can run the protected code
    std::lock_guard<std::mutex> lock(stateMutex);

    deviceStates.clear();
    deviceIdMap.clear();
    nextDeviceId = 1;
}
