#include "RawInputWindow.h"
#include <vector>
#include <cassert>

static RawInputWindow* s_instance = nullptr;

// Static window procedure, dispatches to class instance
LRESULT CALLBACK RawInputWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_INPUT && s_instance && s_instance->inputCallback) {
        UINT dwSize = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
        if (dwSize == 0) return DefWindowProc(hwnd, msg, wParam, lParam);

        std::vector<BYTE> lpb(dwSize);
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize) {
            s_instance->inputCallback(reinterpret_cast<const RAWINPUT*>(lpb.data()));
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool RawInputWindow::Start(std::function<void(const RAWINPUT*)> callback) {
    if (running) return false;
    inputCallback = callback;
    running = true;
    s_instance = this;
    msgThread = std::thread(&RawInputWindow::MessageLoop, this);
    return true;
}

void RawInputWindow::Stop() {
    running = false;
    if (hwnd)
        PostMessageW(hwnd, WM_QUIT, 0, 0);
    if (msgThread.joinable())
        msgThread.join();
    hwnd = nullptr;
    inputCallback = nullptr;
    s_instance = nullptr;
}

void RawInputWindow::MessageLoop() {
    // Register window class
    const wchar_t* className = L"RawInputWindowClass";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = className;
    windowClassAtom = RegisterClassW(&wc);

    hwnd = CreateWindowExW(
        0, className, L"RawInputHiddenWindow",
        0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);

    // Register for raw mouse input
    RAWINPUTDEVICE rid = {};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    threadId = GetCurrentThreadId();

    // Standard Windows message loop
    MSG msg;
    while (running && GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Cleanup
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
    if (windowClassAtom) {
        UnregisterClassW(className, GetModuleHandleW(nullptr));
        windowClassAtom = 0;
    }
}
