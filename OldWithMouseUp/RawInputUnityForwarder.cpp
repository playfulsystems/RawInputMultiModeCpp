#include "RawInputUnityForwarder.h"
#include <Windows.h>

void RawInputUnityForwarder::BufferRawInputEvent(const uint8_t* data, size_t size) {

    // enforce that only one thread at a time can run the protected code
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Find the starting index (in bytes) for this new event in the event buffer
    uint32_t eventStart = (uint32_t)eventBuffer.size();

    // Append the raw input bytes for this event to the buffer
    eventBuffer.insert(eventBuffer.end(), data, data + size);

    // Record where this event's header starts in the buffer
    headerIndices.push_back(eventStart);

    // Record where this event's data (the part after the header) starts in the buffer
    dataIndices.push_back(eventStart + sizeof(RAWINPUTHEADER));

    // Increment the total number of buffered events
    eventCount++;
}

uint32_t RawInputUnityForwarder::GetEventCount() const {

    // enforce that only one thread at a time can run the protected code
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    return eventCount;
}

const uint32_t* RawInputUnityForwarder::GetHeaderIndices() const {

    // enforce that only one thread at a time can run the protected code
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    return headerIndices.data();
}

const uint32_t* RawInputUnityForwarder::GetDataIndices() const {
    std::lock_guard<std::mutex> lock(bufferMutex);
    return dataIndices.data();
}

const uint8_t* RawInputUnityForwarder::GetEventBuffer(int* size) const {
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (size) *size = (int)eventBuffer.size();
    return eventBuffer.data();
}

void RawInputUnityForwarder::Clear() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    eventCount = 0;
    headerIndices.clear();
    dataIndices.clear();
    eventBuffer.clear();
}
