#pragma once
#include <vector>
#include <memory>

namespace TB
{
    enum class CanvasType
    {
        InvalidCanvas,
        WindowsCanvas,
    };

    enum class RendererType
    {
        InvalidRenderer,
        DirectXRenderer,
    };

    struct DataChunk
    {
    public:
        DataChunk()
            : mData(), mSize(0)
        {
        }

        explicit DataChunk(const std::string& str)
            : mData(), mSize(str.size())
        {
            mData.reset(new uint8_t[mSize]);
            memcpy(mData.get(), str.c_str(), mSize);
        }

        DataChunk(std::unique_ptr<uint8_t[]>& data, size_t size)
            : mData(std::move(data)), mSize(size)
        {
        }

        DataChunk(const DataChunk& rhs)
            : mData(nullptr), mSize(rhs.mSize)
        {
            mData.reset(new uint8_t[mSize]);
            memcpy(mData.get(), rhs.mData.get(), mSize);
        }

        DataChunk(DataChunk&& rhs)
            : mData(std::move(rhs.mData)), mSize(rhs.mSize)
        {
        }

        DataChunk& operator=(const DataChunk& rhs)
        {
            mSize = rhs.mSize;
            mData.reset(new uint8_t[mSize]);
            memcpy(mData.get(), rhs.mData.get(), mSize);
            return *this;
        }

        DataChunk& operator=(DataChunk&& rhs)
        {
            mData = std::move(rhs.mData);
            mSize = rhs.mSize;
            return *this;
        }

        bool isValid() const
        {
            return mData != nullptr;
        }

        uint8_t* release()
        {
            return mData.release();
        }

        uint8_t* data() const
        {
            return mData.get();
        }

        size_t size() const
        {
            return mSize;
        }

    private:
        std::unique_ptr<uint8_t[]> mData;
        size_t mSize;
    };

    void runtimeCheck(bool condition);
    void log(const std::string& format, ...);

    std::string normalizePath(const std::string& path);
    std::string operator/(const std::string& path, const std::string& suffix);
    std::string operator~(const std::string& path);
    std::string getCurrentDir();

    std::string loadText(const std::string& path);
    DataChunk loadData(const std::string& path);

    double getElapsedTime();

    enum class Key;
    bool isKeyPressed(const Key& key);
    bool isKeyReleased(const Key& key);

    enum class Mouse;
    bool isMousePressed(const Mouse& button);
    bool isMouseReleased(const Mouse& button);
}

#ifdef _WIN32
#define TB_PLATFORM_WINDOWS 1
#else
#define TB_PLATFORM_WINDOWS 0
#endif

#if TB_PLATFORM_WINDOWS
namespace TB
{
    template<typename PtrType>
    class ComPtr
    {
    public:
        ComPtr()
            : mPtr(nullptr)
        {}

        ComPtr(PtrType* ptr, bool addRef = true)
            : mPtr(ptr)
        {
            if (mPtr && addRef)
            {
                mPtr->AddRef();
            }
        }

        ComPtr(const ComPtr& rhs)
        {
            mPtr = rhs.mPtr;
            if (mPtr)
            {
                mPtr->AddRef();
            }
        }

        ~ComPtr()
        {
            if (mPtr)
            {
                mPtr->Release();
            }
        }

        ComPtr& operator=(PtrType* ptr)
        {
            PtrType* oldPtr = mPtr;
            mPtr = ptr;
            if(mPtr)
            {
                mPtr->AddRef();
            }
            if(oldPtr)
            {
                oldPtr->Release();
            }
            return *this;
        }

        ComPtr& operator=(const ComPtr& rhs)
        {
            return *this = rhs.mPtr;
        }

        bool operator==(const ComPtr& rhs) const
        {
            return mPtr == rhs.mPtr;
        }

        PtrType* operator->() const
        {
            return mPtr;
        }

        operator PtrType*() const
        {
            return mPtr;
        }

        PtrType** getInitRef()
        {
            runtimeCheck(mPtr == nullptr);
            return &mPtr;
        }

        PtrType*& getRef()
        {
            return mPtr;
        }

        PtrType* operator*() const
        {
            return mPtr;
        }

        bool isValid() const
        {
            return mPtr != nullptr;
        }

    private:
        PtrType* mPtr;
    };
}
#endif
