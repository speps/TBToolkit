#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#include <TBToolkit/Common.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <fstream>

namespace
{
    void getMouseState(const TB::Mouse& button, bool& previousState, bool& currentState)
    {
        static std::vector<bool> state;
        static bool initialized = false;
        if (!initialized)
        {
            state.assign((size_t)TB::Mouse::MouseCount, false);
            initialized = true;
        }
        int vkey = 0;
        switch (button)
        {
            case TB::Mouse::Left: vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON; break;
            case TB::Mouse::Right: vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON; break;
            case TB::Mouse::Middle: vkey = VK_MBUTTON; break;
            default: vkey = 0; break;
        }
        previousState = state[(size_t)button];
        currentState = (GetAsyncKeyState(vkey) & 0x8000) != 0;
        state[(size_t)button] = currentState;
    }

    void getKeyState(const TB::Key& key, bool& previousState, bool& currentState)
    {
        static std::vector<bool> state;
        static bool initialized = false;
        if (!initialized)
        {
            state.assign((size_t)TB::Key::KeyCount, false);
            initialized = true;
        }
        int vkey = 0;
        switch (key)
        {
            default: vkey = 0; break;
            case TB::Key::A: vkey = 'A'; break;
            case TB::Key::B: vkey = 'B'; break;
            case TB::Key::C: vkey = 'C'; break;
            case TB::Key::D: vkey = 'D'; break;
            case TB::Key::E: vkey = 'E'; break;
            case TB::Key::F: vkey = 'F'; break;
            case TB::Key::G: vkey = 'G'; break;
            case TB::Key::H: vkey = 'H'; break;
            case TB::Key::I: vkey = 'I'; break;
            case TB::Key::J: vkey = 'J'; break;
            case TB::Key::K: vkey = 'K'; break;
            case TB::Key::L: vkey = 'L'; break;
            case TB::Key::M: vkey = 'M'; break;
            case TB::Key::N: vkey = 'N'; break;
            case TB::Key::O: vkey = 'O'; break;
            case TB::Key::P: vkey = 'P'; break;
            case TB::Key::Q: vkey = 'Q'; break;
            case TB::Key::R: vkey = 'R'; break;
            case TB::Key::S: vkey = 'S'; break;
            case TB::Key::T: vkey = 'T'; break;
            case TB::Key::U: vkey = 'U'; break;
            case TB::Key::V: vkey = 'V'; break;
            case TB::Key::W: vkey = 'W'; break;
            case TB::Key::X: vkey = 'X'; break;
            case TB::Key::Y: vkey = 'Y'; break;
            case TB::Key::Z: vkey = 'Z'; break;
            case TB::Key::Num0: vkey = '0'; break;
            case TB::Key::Num1: vkey = '1'; break;
            case TB::Key::Num2: vkey = '2'; break;
            case TB::Key::Num3: vkey = '3'; break;
            case TB::Key::Num4: vkey = '4'; break;
            case TB::Key::Num5: vkey = '5'; break;
            case TB::Key::Num6: vkey = '6'; break;
            case TB::Key::Num7: vkey = '7'; break;
            case TB::Key::Num8: vkey = '8'; break;
            case TB::Key::Num9: vkey = '9'; break;
            case TB::Key::Escape: vkey = VK_ESCAPE; break;
            case TB::Key::LControl: vkey = VK_LCONTROL; break;
            case TB::Key::LShift: vkey = VK_LSHIFT; break;
            case TB::Key::LAlt: vkey = VK_LMENU; break;
            case TB::Key::LSystem: vkey = VK_LWIN; break;
            case TB::Key::RControl: vkey = VK_RCONTROL; break;
            case TB::Key::RShift: vkey = VK_RSHIFT; break;
            case TB::Key::RAlt: vkey = VK_RMENU; break;
            case TB::Key::RSystem: vkey = VK_RWIN; break;
            case TB::Key::Menu: vkey = VK_APPS; break;
            case TB::Key::LBracket: vkey = VK_OEM_4; break;
            case TB::Key::RBracket: vkey = VK_OEM_6; break;
            case TB::Key::SemiColon: vkey = VK_OEM_1; break;
            case TB::Key::Comma: vkey = VK_OEM_COMMA; break;
            case TB::Key::Period: vkey = VK_OEM_PERIOD; break;
            case TB::Key::Quote: vkey = VK_OEM_7; break;
            case TB::Key::Slash: vkey = VK_OEM_2; break;
            case TB::Key::BackSlash: vkey = VK_OEM_5; break;
            case TB::Key::Tilde: vkey = VK_OEM_3; break;
            case TB::Key::Equal: vkey = VK_OEM_PLUS; break;
            case TB::Key::Dash: vkey = VK_OEM_MINUS; break;
            case TB::Key::Space: vkey = VK_SPACE; break;
            case TB::Key::Return: vkey = VK_RETURN; break;
            case TB::Key::BackSpace: vkey = VK_BACK; break;
            case TB::Key::Tab: vkey = VK_TAB; break;
            case TB::Key::PageUp: vkey = VK_PRIOR; break;
            case TB::Key::PageDown: vkey = VK_NEXT; break;
            case TB::Key::End: vkey = VK_END; break;
            case TB::Key::Home: vkey = VK_HOME; break;
            case TB::Key::Insert: vkey = VK_INSERT; break;
            case TB::Key::Delete: vkey = VK_DELETE; break;
            case TB::Key::Add: vkey = VK_ADD; break;
            case TB::Key::Subtract: vkey = VK_SUBTRACT; break;
            case TB::Key::Multiply: vkey = VK_MULTIPLY; break;
            case TB::Key::Divide: vkey = VK_DIVIDE; break;
            case TB::Key::Left: vkey = VK_LEFT; break;
            case TB::Key::Right: vkey = VK_RIGHT; break;
            case TB::Key::Up: vkey = VK_UP; break;
            case TB::Key::Down: vkey = VK_DOWN; break;
            case TB::Key::Numpad0: vkey = VK_NUMPAD0; break;
            case TB::Key::Numpad1: vkey = VK_NUMPAD1; break;
            case TB::Key::Numpad2: vkey = VK_NUMPAD2; break;
            case TB::Key::Numpad3: vkey = VK_NUMPAD3; break;
            case TB::Key::Numpad4: vkey = VK_NUMPAD4; break;
            case TB::Key::Numpad5: vkey = VK_NUMPAD5; break;
            case TB::Key::Numpad6: vkey = VK_NUMPAD6; break;
            case TB::Key::Numpad7: vkey = VK_NUMPAD7; break;
            case TB::Key::Numpad8: vkey = VK_NUMPAD8; break;
            case TB::Key::Numpad9: vkey = VK_NUMPAD9; break;
            case TB::Key::F1: vkey = VK_F1; break;
            case TB::Key::F2: vkey = VK_F2; break;
            case TB::Key::F3: vkey = VK_F3; break;
            case TB::Key::F4: vkey = VK_F4; break;
            case TB::Key::F5: vkey = VK_F5; break;
            case TB::Key::F6: vkey = VK_F6; break;
            case TB::Key::F7: vkey = VK_F7; break;
            case TB::Key::F8: vkey = VK_F8; break;
            case TB::Key::F9: vkey = VK_F9; break;
            case TB::Key::F10: vkey = VK_F10; break;
            case TB::Key::F11: vkey = VK_F11; break;
            case TB::Key::F12: vkey = VK_F12; break;
            case TB::Key::F13: vkey = VK_F13; break;
            case TB::Key::F14: vkey = VK_F14; break;
            case TB::Key::F15: vkey = VK_F15; break;
            case TB::Key::Pause: vkey = VK_PAUSE; break;
        }
        previousState = state[(size_t)key];
        currentState = (GetAsyncKeyState(vkey) & 0x8000) != 0;
        state[(size_t)key] = currentState;
    }
}

namespace TB
{
    void runtimeCheck(bool condition)
    {
        if (!condition)
        {
            __debugbreak();
        }
    }

    void log(const std::string& format, ...)
    {
        va_list va;
        va_start(va, format);
        int len = _vscprintf(format.c_str(), va) + 1;
        std::unique_ptr<char> buffer(new char[len]);
        vsprintf_s(buffer.get(), len, format.c_str(), va);
        va_end(va);
        OutputDebugStringA(buffer.get());
    }

    std::string normalizePath(const std::string& path)
    {
        auto result = path;
        std::transform(result.begin(), result.end(), result.begin(), [](const char& c) { return c == '\\' ? '/' : c; });
        for (size_t i = 0; i < result.size() - 1; i++)
        {
            if (result[i] == '/' && result[i] == result[i+1])
            {
                result.erase(i--, 1);
            }
        }
        return result;
    }

    std::string operator/(const std::string& path, const std::string& suffix)
    {
        auto result = normalizePath(path);
        if (result.back() == '/')
        {
            result.pop_back();
        }
        result += '/';
        auto rest = suffix;
        if (rest.back() == '/')
        {
            rest.pop_back();
        }
        return result + rest;
    }

    std::string operator~(const std::string& path)
    {
        auto result = normalizePath(path);
        while (result.back() != '/')
        {
            result.pop_back();
        }
        return result;
    }

    std::string getCurrentDir()
    {
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        return std::string(currentDir);
    }

    std::string loadText(const std::string& path)
    {
        auto fullPath = getCurrentDir() / path;
        std::ifstream stream;
        stream.open(fullPath, std::ios::in);
        runtimeCheck(stream.is_open());

        stream.seekg(0, stream.end);
        size_t length = stream.tellg();
        stream.seekg(0, stream.beg);
 
        std::string result;
        result.resize(length, ' '); // reserve space
        char* begin = &*result.begin();
        stream.read(begin, length);

        stream.close();

        return result;
    }

    DataChunk loadData(const std::string& path)
    {
        auto fullPath = getCurrentDir() / path;
        std::ifstream stream;
        stream.open(fullPath, std::ios::in | std::ios::binary);
        runtimeCheck(stream.is_open());

        stream.seekg(0, stream.end);
        size_t length = stream.tellg();
        stream.seekg(0, stream.beg);
 
        std::unique_ptr<uint8_t[]> data(new uint8_t[length]);
        stream.read((char*)data.get(), length);

        stream.close();

        return DataChunk(data, length);
    }

    double getElapsedTime()
    {
        static LARGE_INTEGER freq;
        static bool initialized = false;
        if (!initialized)
        {
            QueryPerformanceFrequency(&freq);
            initialized = true;
        }

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);

        return (double)counter.QuadPart / freq.QuadPart;
    }

    bool isKeyPressed(const Key& key)
    {
        bool previousState = false;
        bool currentState = false;
        getKeyState(key, previousState, currentState);
        return currentState && !previousState;
    }

    bool isKeyReleased(const Key& key)
    {
        bool previousState = false;
        bool currentState = false;
        getKeyState(key, previousState, currentState);
        return !currentState && previousState;
    }

    bool isMousePressed(const Mouse& button)
    {
        bool currentState = false;
        bool previousState = false;
        getMouseState(button, previousState, currentState);
        return currentState && !previousState;
    }

    bool isMouseReleased(const Mouse& button)
    {
        bool currentState = false;
        bool previousState = false;
        getMouseState(button, previousState, currentState);
        return !currentState && previousState;
    }
}

#endif
