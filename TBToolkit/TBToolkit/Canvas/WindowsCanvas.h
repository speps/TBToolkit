#pragma once

#include <TBToolkit/Canvas.h>
#include <memory>
#include <string>

#if TB_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace TB
{
    class WindowsCanvas : public Canvas
    {
    public:
        WindowsCanvas(HINSTANCE hInstance);
        virtual ~WindowsCanvas();

        virtual CanvasType getType() const override { return CanvasType::WindowsCanvas; }
        virtual bool init(const std::wstring& title, int width, int height) override;

        HWND getHwnd() const;
        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        static bool mInitialized;

        HINSTANCE mHinstance;
        HWND mHwnd;
    };
}

#endif