#pragma once
#include <TBToolkit/Platform.h>

#include <memory>
#include <string>

namespace TB
{
    class Canvas
    {
    public:
        Canvas() {}
        virtual ~Canvas() {}

        virtual CanvasType getType() const = 0;
        virtual bool init(const std::wstring& title, int width, int height) = 0;
    };

#ifdef TB_PLATFORM_WINDOWS
    std::shared_ptr<Canvas> CreateWindowsCanvas(void* hInstance);
#endif
}
