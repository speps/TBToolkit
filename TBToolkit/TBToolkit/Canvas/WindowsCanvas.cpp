#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Canvas/WindowsCanvas.h>
#include <memory>
#include <string>
#include <objbase.h>

namespace TB
{
    bool WindowsCanvas::mInitialized = false;

    WindowsCanvas::WindowsCanvas(HINSTANCE hInstance)
        : mHinstance(hInstance)
        , mHwnd(0)
    {
    }

    WindowsCanvas::~WindowsCanvas()
    {
    }

    bool WindowsCanvas::init(const std::wstring& title, int width, int height)
    {
        if (mHwnd != 0)
        {
            return false;
        }

        if (!mInitialized)
        {
            if (CoInitializeEx(nullptr, COINIT_MULTITHREADED) != S_OK)
            {
                return false;
            }
            mInitialized = true;
        }

        {
            // Register class
            WNDCLASSEX wcex;
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WndProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = mHinstance;
            wcex.hIcon = 0;
            wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = L"TBToolkitWindowClass";
            wcex.hIconSm = 0;
            if (!RegisterClassEx(&wcex))
            {
                return false;
            }

            // Create window
            RECT rc = { 0, 0, width, height };
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
            mHwnd = CreateWindow(L"TBToolkitWindowClass", title.c_str(), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, mHinstance, nullptr);
            if (mHwnd == 0)
            {
                return false;
            }

            SetWindowLongPtr(mHwnd, GWLP_USERDATA, (LONG_PTR)this);

            ShowWindow(mHwnd, SW_SHOW);
        }

        return true;
    }

    HWND WindowsCanvas::getHwnd() const
    {
        return mHwnd;
    }

    LRESULT CALLBACK WindowsCanvas::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (message)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    std::shared_ptr<Canvas> CreateWindowsCanvas(void* hInstance)
    {
        return std::make_shared<WindowsCanvas>((HINSTANCE)hInstance);
    }
}

#endif