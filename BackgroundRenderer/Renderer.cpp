#include "pch.h"
#include "Renderer.h"

#include <mfmediaengine.h>
#include <mfapi.h>
#include <d3d11.h>

#define RETURN_ON_ERROR(expr) {                                                \
    HRESULT _errorCode = expr;                                                 \
    if(FAILED(_errorCode)) {                                                   \
        return;                                                                \
    }                                                                          \
}
//URHO3D_LOGERRORF("VideoPlayer error in '%s' 0x%x", #expr, _errorCode); 

using namespace BackgroundRenderer;
using namespace Platform;

class MediaEngineNotify : public IMFMediaEngineNotify
{
public:
    MediaEngineNotify() :
        refCount(1)
    {}

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0) delete this;
        return count;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppv)
    {
        if (!ppv) return E_POINTER;

        if (iid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IUnknown*>(this);
        }
        else if (iid == __uuidof(IMFMediaEngineNotify))
        {
            *ppv = static_cast<IMFMediaEngineNotify*>(this);
        }
        else
        {
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE EventNotify(DWORD event, DWORD_PTR param1, DWORD param2)
    {
        switch (event)
        {
        case MF_MEDIA_ENGINE_EVENT_ERROR:
            OutputDebugString(L"Video player error");
            //URHO3D_LOGERRORF("VideoPlayer error 0x%x", param2);
            break;
        }
        return S_OK;
    }
private:
    ULONG refCount;
};

static CComPtr<IMFDXGIDeviceManager> GetDxgiManager(ID3D11Device* dxDevice)
{
    static CComPtr<IMFDXGIDeviceManager> dxgiManager;
    if (!dxgiManager)
    {
        //ID3D11Device* dxDevice = context->GetSubsystem<Urho3D::Graphics>()->GetImpl()->GetDevice();
        CComQIPtr<ID3D10Multithread> dxMultithread = dxDevice;
        dxMultithread->SetMultithreadProtected(TRUE);
        UINT resetToken = 0;
        MFCreateDXGIDeviceManager(&resetToken, &dxgiManager);
        dxgiManager->ResetDevice(dxDevice, resetToken);
    }
    return dxgiManager;
}

Renderer::Renderer(uint64 ptr, uint64 texPtr)
{

    ID3D11Device * dxDevice = (ID3D11Device *)(void *)ptr;

    RETURN_ON_ERROR(MFStartup(MF_VERSION));
    CComPtr<IMFMediaEngineClassFactory> mediaEngineFactory;
    RETURN_ON_ERROR(CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&mediaEngineFactory)));

    CComPtr<IMFAttributes> attributes;
    RETURN_ON_ERROR(MFCreateAttributes(&attributes, 3));
    RETURN_ON_ERROR(attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, new MediaEngineNotify()));
    RETURN_ON_ERROR(attributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, DXGI_FORMAT_B8G8R8X8_UNORM));
    RETURN_ON_ERROR(attributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, GetDxgiManager(dxDevice)));
    RETURN_ON_ERROR(mediaEngineFactory->CreateInstance(0, attributes, &mediaEngine_));

    //SubscribeToEvent(Urho3D::E_RENDERSURFACEUPDATE, URHO3D_HANDLER(VideoPlayer, HandleRenderSurfaceUpdate));

    std::wstring url(L"C:\\Users\\Owner\\AppData\\Local\\Packages\\4953a40c-1620-4361-b04c-52485940c8c3_z10v2vk4de6hc\\LocalState\\Bunny.mp4");
    BSTR sourceBSTR;
    sourceBSTR = SysAllocString(url.c_str());
    auto hr = mediaEngine_->SetSource(sourceBSTR);
    mediaEngine_->Play();
    SysFreeString(sourceBSTR);
}

void Renderer::Update(uint64 texPtr)
{
    LONGLONG pts;
    if (mediaEngine_->OnVideoStreamTick(&pts) != S_OK) return;

    //ResizeTexture();
    //Urho3D::RenderSurface* renderSurface = texture_->GetRenderSurface();
    //if (!renderSurface || !renderSurface->IsUpdateQueued()) return;

    RECT rect = { 0, 0, 320, 180 };
    MFARGB borderColor = { 0, 0, 0, 255 };
    ID3D11Texture2D* dxTexture = (ID3D11Texture2D*)(void*) texPtr;
    RETURN_ON_ERROR(mediaEngine_->TransferVideoFrame(dxTexture, nullptr, &rect, &borderColor));
    //if (generateMipmaps_) texture_->RegenerateLevels();
    //renderSurface->ResetUpdateQueued();
}
