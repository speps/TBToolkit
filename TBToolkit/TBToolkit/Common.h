#pragma once

#include <TBToolkit/Platform.h>

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

        ComPtr(PtrType* ptr,bool addRef = true)
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
