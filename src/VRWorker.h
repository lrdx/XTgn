#ifndef __VRWorker_H__
#define __VRWorker_H__

#include "logger.h"

#include <memory>
#include <thread>
#include <wrl\client.h>

#include "d3d11.h"

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

struct openvr_context
{
	ID3D11Device* dev11;
	ID3D11DeviceContext* ctx11;
	ID3D11Resource* tex;
	ID3D11ShaderResourceView* mirrorSrv;

	int width;
	int height;
};

class VRWorker
{
public:
	VRWorker(Logger* logger);
	~VRWorker();

	void Initalize();
	void Release();

	const bool IsInitialized() { return m_initialized; }
	const std::shared_ptr<uint8_t*> GetBuffer() { return m_buffer; }
	const int GetWidth() { return m_vr_context->width; }
	const int GetHeight() { return m_vr_context->height; }

	bool CopyScreenToBuffer();

private:
	Logger* m_logger;

	bool m_initialized;
	std::shared_ptr<uint8_t*> m_buffer;

	openvr_context* m_vr_context;

	HRESULT CaptureTexture(
		_In_ ID3D11DeviceContext* pContext,
		_In_ ID3D11Resource* pSource,
		D3D11_TEXTURE2D_DESC& desc,
		Microsoft::WRL::ComPtr<ID3D11Texture2D>& pStaging);

	size_t BitsPerPixel(_In_ DXGI_FORMAT fmt);

	HRESULT GetSurfaceInfo(
		_In_ size_t width,
		_In_ size_t height,
		_In_ DXGI_FORMAT fmt,
		_Out_opt_ size_t* outNumBytes,
		_Out_opt_ size_t* outRowBytes,
		_Out_opt_ size_t* outNumRows);
};

#endif //__VRWorker_H__