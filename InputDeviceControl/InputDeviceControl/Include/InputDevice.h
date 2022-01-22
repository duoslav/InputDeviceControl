#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <cstdint>
#include <thread>

class InputDevice
{
public:
    class BaseHook
    {
    private:
        HHOOK mHookHandler;
        std::thread mThread;
        volatile bool mIsHookActive;
        int mHookId;
        HOOKPROC mHookProc;

    public:
        BaseHook(int aHookId, HOOKPROC aHookProc) :
            mHookId(aHookId),
            mHookProc(aHookProc)
        {
            mThread = std::thread(Loop, this);
        }

        static void Loop(BaseHook* aThis)
        {
            MSG msg;

            aThis->mHookHandler = SetWindowsHookEx(aThis->mHookId, aThis->mHookProc, GetModuleHandle(0), 0);

            aThis->mIsHookActive = true;
            while (aThis->mIsHookActive)
            {
                while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                    DispatchMessage(&msg);
            }
        }

        void Stop()
        {
            mIsHookActive = false;
        }

        ~BaseHook()
        {
            mThread.join();
            UnhookWindowsHookEx(mHookHandler);
        }
    };

public:
    static void Block()
    {
        BlockInput(TRUE);
    }

    static void Unblock()
    {
        BlockInput(FALSE);
    }

    static void ToClipboardWStr(const wchar_t* strData) {
        if (OpenClipboard(0)) {
            EmptyClipboard();
            size_t size_m = sizeof(WCHAR) * (wcslen(strData) + 1);
            HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, size_m);
            WCHAR* pchData;
            pchData = (WCHAR*)GlobalLock(hClipboardData);
            wcscpy_s(pchData, size_m / sizeof(wchar_t), strData);
            GlobalUnlock(hClipboardData);
            SetClipboardData(CF_UNICODETEXT, hClipboardData);
            CloseClipboard();
        }
    }

    static wchar_t * const GetLastErrorMessage()
    {
        static wchar_t buf[256];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
        return buf;
    }
};


class Mouse : public InputDevice
{
public:
    static void LClick(uint32_t x, uint32_t y)
    {
        SetCursorPos(x, y);
        LClick();
    }

    static void RClick(uint32_t x, uint32_t y)
    {
        SetCursorPos(x, y);
        RClick();
    }

    static void SetPos(uint32_t x, uint32_t y)
    {
        SetCursorPos(x, y);
    }

    static void LClick()
    {
        INPUT Inputs[2] = { 0 };

        Inputs[0].type = INPUT_MOUSE;
        Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

        Inputs[1].type = INPUT_MOUSE;
        Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

        SendInput(2, Inputs, sizeof(INPUT));
    }

    static void RClick()
    {
        INPUT Inputs[2] = { 0 };

        Inputs[0].type = INPUT_MOUSE;
        Inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

        Inputs[1].type = INPUT_MOUSE;
        Inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;

        SendInput(2, Inputs, sizeof(INPUT));
    }
};

class Keyboard : public InputDevice
{
public:
    class Hook : public BaseHook
    {
    public:
        Hook(HOOKPROC aHookProc) : BaseHook(WH_KEYBOARD_LL, aHookProc) {}

    public:
    };

public:
    static void Push(const char aKey)
    {
        // Create a generic keyboard event structure
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        // Press the "X" key
        ip.ki.wVk = aKey;
        ip.ki.dwFlags = 0;
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "X" key
        ip.ki.wVk = aKey;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }

    static void CtrlPlus(const char aKey)
    {
        // Create a generic keyboard event structure
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        // Press the "Ctrl" key
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = 0;
        SendInput(1, &ip, sizeof(INPUT));

        // Press the "X" key
        ip.ki.wVk = aKey;
        ip.ki.dwFlags = 0;
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "X" key
        ip.ki.wVk = aKey;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "Ctrl" key
        ip.ki.wVk = VK_CONTROL;
        ip.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }

};
