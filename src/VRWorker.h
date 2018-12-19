#ifndef __VRWORKER_H__
#define __VRWORKER_H__

#include "Logger.h"

#include <memory>

#include <wrl\client.h>
#include "d3d11.h"
#include <openvr.h>

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

	void Initalize(vr::EVREye eye);
	void Release();

	bool IsInitialized() const { return m_initialized; }

	uint8_t* GetBuffer() const { return m_buffer.get(); }	
	size_t GetBufferRowCount() const { return bufferRowCount; }
	size_t GetBufferRowPitch() const { return bufferRowPitch; }
	int GetWidth() const { return m_vr_context->width; }
	int GetHeight() const { return m_vr_context->height; }

	DXGI_FORMAT GetFormat() const { return m_format; }

	bool CopyScreenToBuffer();

private:
	Logger* m_logger;

	bool m_initialized;
	size_t bufferRowCount;
	size_t bufferRowPitch;
	std::unique_ptr<uint8_t[]> m_buffer;

	DXGI_FORMAT m_format;

	std::unique_ptr<openvr_context> m_vr_context;

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

#endif //__VRWORKER_H__