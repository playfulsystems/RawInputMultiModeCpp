#pragma once
// RawInputUnityForwarder.h
// Buffers raw input events for Unity's ForwardRawInput system

#include <vector>
#include <mutex>
#include <cstdint>

class RawInputUnityForwarder {
public:
    // Adds a new raw input event to buffer (called from window proc)
    void BufferRawInputEvent(const uint8_t* data, size_t size);

    // --- Unity API accessors ---
    uint32_t GetEventCount() const;
    const uint32_t* GetHeaderIndices() const;
    const uint32_t* GetDataIndices() const;
    const uint8_t* GetEventBuffer(int* size) const;

    // Clears all buffers and resets event count
    void Clear();

private:
    std::vector<uint8_t> eventBuffer;
    std::vector<uint32_t> headerIndices, dataIndices;
    uint32_t eventCount = 0;
    mutable std::mutex bufferMutex;
};
