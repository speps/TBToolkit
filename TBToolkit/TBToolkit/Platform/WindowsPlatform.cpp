#include <TBToolkit/Platform.h>
#if TB_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <fstream>

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
 
        std::unique_ptr<uint8_t> data(new uint8_t[length]);
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
}

#endif
