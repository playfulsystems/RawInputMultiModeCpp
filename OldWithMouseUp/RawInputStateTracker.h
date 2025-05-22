#pragma once
// RawInputStateTracker.h
// Manages multi-mouse device state tracking for polling from Unity

#include <Windows.h>
#include <map>
#include <mutex>
#include "RawInputPlugin.h" // For RawMouseState and constants

class RawInputStateTracker {
public:
    // Handles a RAWINPUT mouse event, updating tracked state for that device
    void HandleRawInput(const RAWINPUT* raw);

    // Fills provided buffer with state of all tracked devices; returns count written
    int PollMouseStates(RawMouseState* buffer, int bufferSize);

    // Clears all tracked device state (used on shutdown/reset)
    void Reset();

private:
    std::map<HANDLE, int> deviceIdMap;
    std::map<HANDLE, RawMouseState> deviceStates;
    int nextDeviceId = 1;
    std::mutex stateMutex;
};
