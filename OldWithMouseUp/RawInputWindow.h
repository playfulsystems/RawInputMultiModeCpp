#pragma once
// RawInputWindow.h
// Manages hidden message-only window and Windows message loop for raw input

#include <Windows.h>
#include <thread>
#include <atomic>
#include <functional>

class RawInputWindow {
public:
    // Starts the message thread; calls inputCallback on WM_INPUT messages
    bool Start(std::function<void(const RAWINPUT*)> inputCallback);

    // Stops the message loop and cleans up the window/thread
    void Stop();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void MessageLoop();

    HWND hwnd = nullptr;
    std::thread msgThread;
    std::atomic<bool> running{ false };
    std::function<void(const RAWINPUT*)> inputCallback;
    ATOM windowClassAtom = 0;
    DWORD threadId = 0;
};
