#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <stdint.h>
#include <memory>

#include "d3d11.h"

#include <libavutil/pixfmt.h>

class FrameBuffer
{
public:
	FrameBuffer(int w, int h, AVPixelFormat f)
		: width(w), height(h), fmt(f)
	{
		//buffer = std::make_shared<uint8_t>(new uint8_t(w * h * f);
	}

	FrameBuffer(int w, int h, int row, DXGI_FORMAT f)
		: width(w), height(h)
	{
		size_t rowPitch, slicePitch, rowCount;
		FrameBuffer::GetSurfaceInfo(w, h, f, &slicePitch, &rowPitch, &rowCount);

		buffer = std::make_shared<uint8_t>(new uint8_t(slicePitch));
		rowPitch = rowCount;
	}

	int width;
	int height;
	int rowPitch;
	AVPixelFormat fmt;
	std::shared_ptr<uint8_t> buffer;

	static HRESULT FrameBuffer::GetSurfaceInfo(
		_In_ size_t width,
		_In_ size_t height,
		_In_ DXGI_FORMAT fmt,
		_Out_opt_ size_t* outNumBytes,
		_Out_opt_ size_t* outRowBytes,
		_Out_opt_ size_t* outNumRows);
};

#endif //__FRAME_BUFFER_H__