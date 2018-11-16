#include "VRWorker.h"

#include <wrl\client.h>
#include <d3d11.h>

#include "openvr.h"

#pragma comment(lib, "d3d11.lib")

#pragma comment(lib, "win64/openvr_api.lib")

VRWorker::VRWorker(Logger* logger)
	: m_logger(logger), m_initialized(false)
{
	m_vr_context = std::make_unique<openvr_context>();

	m_vr_context->ctx11 = nullptr;
	m_vr_context->dev11 = nullptr;
	m_vr_context->mirrorSrv = nullptr;
	m_vr_context->tex = nullptr;
	m_vr_context->width = 0;
	m_vr_context->height = 0;
}

VRWorker::~VRWorker()
{
	Release();
}

void VRWorker::Initalize(const bool isRightEye)
{
	if (m_initialized)
		Release();

	m_logger->WriteInfo("Try to initialize OpenVR...");

	// Init OpenVR, create D3D11 device and get shared mirror texture
	vr::EVRInitError err = vr::VRInitError_None;
	vr::VR_Init(&err, vr::VRApplication_Background);

	if (err != vr::VRInitError_None)
	{
		m_logger->WriteError("OpenVR not available\r\n");
		return;
	}

	D3D_FEATURE_LEVEL featureLevel;
	if (!SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &m_vr_context->dev11, &featureLevel, &m_vr_context->ctx11)))
	{
		m_logger->WriteError("D3D11CreateDevice failed\r\n");
		return;
	}

	const auto err_comp = vr::VRCompositor()->GetMirrorTextureD3D11(isRightEye ? vr::Eye_Right : vr::Eye_Left, m_vr_context->dev11, reinterpret_cast<void**>(&m_vr_context->mirrorSrv));
	if (err_comp != vr::EVRCompositorError::VRCompositorError_None || !m_vr_context->mirrorSrv)
	{
		m_logger->WriteError("GetMirrorTextureD3D11 failed\r\n");
		return;
	}

	// Get ID3D11Resource from shader resource view
	m_vr_context->mirrorSrv->GetResource(&m_vr_context->tex);
	if (!m_vr_context->tex)
	{
		m_logger->WriteError("GetResource failed\r\n");
		return;
	}

	// Get the size from Texture2D
	ID3D11Texture2D* tex2D;
	
	if (!SUCCEEDED(m_vr_context->tex->QueryInterface<ID3D11Texture2D>(&tex2D)) || !tex2D)
	{
		m_logger->WriteError("QueryInterface failed\r\n");
		return;
	}

	D3D11_TEXTURE2D_DESC desc;
	tex2D->GetDesc(&desc);

	m_format = desc.Format;

	m_vr_context->width = desc.Width;
	m_vr_context->height = desc.Height;

	size_t rowPitch, slicePitch, rowCount;
	if (!SUCCEEDED(GetSurfaceInfo(desc.Width, desc.Height, desc.Format, &slicePitch, &rowPitch, &rowCount)))
	{
		return;
	}

	m_buffer = std::make_unique<uint8_t[]>(slicePitch);
	bufferRowCount = rowCount;
	bufferRowPitch = rowPitch;

	tex2D->Release();

	m_logger->WriteError("Successful initalization OpenVR\r\n");

	m_initialized = true;
}

void VRWorker::Release()
{
	m_initialized = false;

	if (m_vr_context->tex)
		m_vr_context->tex->Release();

	if (m_vr_context->mirrorSrv)
	{
		//vr::VRCompositor()->ReleaseMirrorTextureD3D11(m_vr_context->mirrorSrv);
		m_vr_context->mirrorSrv->Release();
	}

	if (m_vr_context->ctx11)
		m_vr_context->ctx11->Release();

	if (m_vr_context->dev11)
		m_vr_context->dev11->Release();

	m_vr_context->ctx11 = nullptr;
	m_vr_context->dev11 = nullptr;
	m_vr_context->mirrorSrv = nullptr;
	m_vr_context->tex = nullptr;
}

bool VRWorker::CopyScreenToBuffer()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pStaging;
	D3D11_TEXTURE2D_DESC desc = {};
	CaptureTexture(m_vr_context->ctx11, m_vr_context->tex, desc, pStaging);

	D3D11_MAPPED_SUBRESOURCE mapped;
	auto hr = m_vr_context->ctx11->Map(pStaging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hr))
	{
		return false;
	}

	auto sptr = static_cast<const char*>(mapped.pData);
	if (!sptr)
	{
		m_vr_context->ctx11->Unmap(pStaging.Get(), 0);
		return false;
	}

	size_t rowPitch, slicePitch, rowCount;
	hr = GetSurfaceInfo(desc.Width, desc.Height, desc.Format, &slicePitch, &rowPitch, &rowCount);
	if (FAILED(hr))
	{
		m_vr_context->ctx11->Unmap(pStaging.Get(), 0);
		return false;
	}
	auto startTime = std::chrono::high_resolution_clock::now();
	uint8_t* dptr = m_buffer.get();
	size_t msize = std::min<size_t>(rowPitch, mapped.RowPitch);
	for (size_t h = 0; h < rowCount; ++h)
	{
		memcpy_s(dptr, rowPitch, sptr, msize);
		sptr += mapped.RowPitch;
		dptr += rowPitch;
	}

	auto endTime = std::chrono::high_resolution_clock::now();

	auto lastedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	//m_logger->WriteInfo(QString("Copy screen to buffer lasted %1 ms\r\n").arg(lastedTime));
	m_vr_context->ctx11->Unmap(pStaging.Get(), 0);

	return true;
}

HRESULT VRWorker::CaptureTexture(
	_In_ ID3D11DeviceContext* pContext,
	_In_ ID3D11Resource* pSource,
	D3D11_TEXTURE2D_DESC& desc,
	Microsoft::WRL::ComPtr<ID3D11Texture2D>& pStaging)
{
	if (!pContext || !pSource)
		return E_INVALIDARG;

	D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	pSource->GetType(&resType);

	if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
	HRESULT hr = pSource->QueryInterface(IID_GRAPHICS_PPV_ARGS(pTexture.GetAddressOf()));
	if (FAILED(hr))
		return hr;

	assert(pTexture);

	pTexture->GetDesc(&desc);

	if (desc.ArraySize > 1 || desc.MipLevels > 1)
	{
	}

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	pContext->GetDevice(d3dDevice.GetAddressOf());

	if (desc.SampleDesc.Count > 1)
	{
		// MSAA content must be resolved before being copied to a staging texture
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTemp;
		hr = d3dDevice->CreateTexture2D(&desc, nullptr, pTemp.GetAddressOf());
		if (FAILED(hr))
			return hr;

		assert(pTemp);

		UINT support = 0;
		if (FAILED(hr))
			return hr;

		if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE))
			return E_FAIL;

		for (UINT item = 0; item < desc.ArraySize; ++item)
		{
			for (UINT level = 0; level < desc.MipLevels; ++level)
			{
				UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
				//pContext->ResolveSubresource(pTemp.Get(), index, pSource, index, fmt);
			}
		}

		desc.BindFlags = 0;
		desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;

		hr = d3dDevice->CreateTexture2D(&desc, nullptr, pStaging.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return hr;

		assert(pStaging);

		pContext->CopyResource(pStaging.Get(), pTemp.Get());
	}
	else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
	{
		// Handle case where the source is already a staging texture we can use directly
		pStaging = pTexture;
	}
	else
	{
		// Otherwise, create a staging texture from the non-MSAA source
		desc.BindFlags = 0;
		desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;

		hr = d3dDevice->CreateTexture2D(&desc, nullptr, pStaging.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return hr;

		assert(pStaging);

		pContext->CopyResource(pStaging.Get(), pSource);
	}


	return S_OK;
}

size_t VRWorker::BitsPerPixel(_In_ DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 24;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)

	case DXGI_FORMAT_V408:
		return 24;

	case DXGI_FORMAT_P208:
	case DXGI_FORMAT_V208:
		return 16;

#endif // (_WIN32_WINNT >= _WIN32_WINNT_WIN10)

#if defined(_XBOX_ONE) && defined(_TITLE)

	case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
	case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
	case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
		return 32;

	case DXGI_FORMAT_D16_UNORM_S8_UINT:
	case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		return 24;

	case DXGI_FORMAT_R4G4_UNORM:
		return 8;

#endif // _XBOX_ONE && _TITLE

	default:
		return 0;
	}
}

HRESULT VRWorker::GetSurfaceInfo(
	_In_ size_t width,
	_In_ size_t height,
	_In_ DXGI_FORMAT fmt,
	_Out_opt_ size_t* outNumBytes,
	_Out_opt_ size_t* outRowBytes,
	_Out_opt_ size_t* outNumRows)
{
	uint64_t numBytes = 0;
	uint64_t rowBytes = 0;
	uint64_t numRows = 0;

	bool bc = false;
	bool packed = false;
	bool planar = false;
	size_t bpe = 0;
	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bc = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		bc = true;
		bpe = 16;
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		packed = true;
		bpe = 4;
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		packed = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
	case DXGI_FORMAT_P208:
#endif
		planar = true;
		bpe = 2;
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		planar = true;
		bpe = 4;
		break;

#if defined(_XBOX_ONE) && defined(_TITLE)

	case DXGI_FORMAT_D16_UNORM_S8_UINT:
	case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		planar = true;
		bpe = 4;
		break;

#endif

	default:
		break;
	}

	if (bc)
	{
		uint64_t numBlocksWide = 0;
		if (width > 0)
		{
			numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
		}
		uint64_t numBlocksHigh = 0;
		if (height > 0)
		{
			numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
		}
		rowBytes = numBlocksWide * bpe;
		numRows = numBlocksHigh;
		numBytes = rowBytes * numBlocksHigh;
	}
	else if (packed)
	{
		rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
		numRows = uint64_t(height);
		numBytes = rowBytes * height;
	}
	else if (fmt == DXGI_FORMAT_NV11)
	{
		rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
		numRows = uint64_t(height) * 2u; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
		numBytes = rowBytes * numRows;
	}
	else if (planar)
	{
		rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
		numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
		numRows = height + ((uint64_t(height) + 1u) >> 1);
	}
	else
	{
		size_t bpp = BitsPerPixel(fmt);
		if (!bpp)
			return E_INVALIDARG;

		rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // round up to nearest byte
		numRows = uint64_t(height);
		numBytes = rowBytes * height;
	}

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
	static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
	if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
#else
	static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

	if (outNumBytes)
	{
		*outNumBytes = static_cast<size_t>(numBytes);
	}
	if (outRowBytes)
	{
		*outRowBytes = static_cast<size_t>(rowBytes);
	}
	if (outNumRows)
	{
		*outNumRows = static_cast<size_t>(numRows);
	}

	return S_OK;
}
