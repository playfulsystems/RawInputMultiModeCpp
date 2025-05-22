#include "RawInputPlugin.h"
#include "RawInputStateTracker.h"
#include "RawInputUnityForwarder.h"
#include "RawInputWindow.h"
#include <atomic>

// --- Static plugin state (private to this translation unit) ---
static RawInputStateTracker g_stateTracker;
static RawInputUnityForwarder g_forwarder;
static RawInputWindow g_window;
static std::atomic<bool> g_forwardToUnity{ true };
static std::atomic<bool> g_running{ false };

// --- Helper: Called on each WM_INPUT event ---
static void OnRawInputEvent(const RAWINPUT* raw) {
    // Forward raw data to both systems as needed
    if (g_forwardToUnity)
        g_forwarder.BufferRawInputEvent(reinterpret_cast<const uint8_t*>(raw), sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE));
    g_stateTracker.HandleRawInput(raw);
}

// --- DLL Exported API ---

extern "C" {

    // --- Multi-mouse poll for Unity ---
    DLLEXPORT int PollRawMouseInput(RawMouseState* buffer, int bufferSize) {
        return g_stateTracker.PollMouseStates(buffer, bufferSize);
    }

    DLLEXPORT void StartRawInputThread() {
        if (!g_running.exchange(true)) {
            g_window.Start(OnRawInputEvent);
        }
    }

    DLLEXPORT void StopRawInputThread() {
        if (g_running.exchange(false)) {
            g_window.Stop();
            g_stateTracker.Reset();
            g_forwarder.Clear();
        }
    }

    DLLEXPORT void SetForwardToUnity(bool enable) {
        g_forwardToUnity = enable;
    }

    // --- Unity-style raw input forwarding API ---
    DLLEXPORT uint32_t GetRawInputEventCount() {
        return g_forwarder.GetEventCount();
    }

    DLLEXPORT const uint32_t* GetRawInputHeaderIndices() {
        return g_forwarder.GetHeaderIndices();
    }

    DLLEXPORT const uint32_t* GetRawInputDataIndices() {
        return g_forwarder.GetDataIndices();
    }

    DLLEXPORT const uint8_t* GetRawInputDataBuffer(int* size) {
        return g_forwarder.GetEventBuffer(size);
    }

    DLLEXPORT void ClearRawInputEventBuffers() {
        g_forwarder.Clear();
    }

}

