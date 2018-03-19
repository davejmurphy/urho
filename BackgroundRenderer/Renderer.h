#pragma once

#include <atlbase.h>

struct IMFMediaEngine;

namespace BackgroundRenderer
{
    public ref class Renderer sealed
    {
    public:
        Renderer(uint64 ptr, uint64 texPtr);

        void Update(uint64 texPtr);
    private:
        CComPtr<IMFMediaEngine> mediaEngine_;
    };
}
