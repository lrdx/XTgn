#ifndef __XVIDEO_WRITER_H__
#define __XVIDEO_WRITER_H__

#include "Logger.h"

#include "d3d11.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus 
}
#endif

struct ffmpeg_context
{
	AVCodec* codec;
	AVFrame* frame;
	AVFrame* tmp_frame;
	AVCodecContext* ctx;
	AVPacket* pkt;
	int frame_pts;
	FILE* file;
	struct SwsContext* sws_ctx;
};

class XVideoWriter
{
public:
	static std::vector<std::string> GetAllEncoders();

public:
	XVideoWriter(Logger* logger);
	~XVideoWriter();

	bool IsInitialized() const { return m_initialized; }

	void Initialize(const std::string& filename,
		const std::string& codec_name,
		int videoBitrate,
		int videoWidth,
		int videoHeight,
		int videoFramerate,
		DXGI_FORMAT format,
		int height,
		int width);
	void Initialize(const std::string& filename,
		const std::string& codec_name,
		int videoBitrate,
		int videoWidth,
		int videoHeight,
		int videoFramerate,
		AVPixelFormat format,
		int height,
		int width);
	void Release();
	void WriteFrame(uint8_t* buf, int rowCount, int rowPitch);
	void CloseFile();

private:
	Logger* m_logger;

	std::unique_ptr<ffmpeg_context> m_video_context;
	
	bool m_initialized;

	void CopyBufferWithSws(uint8_t* buf, int rowCount, int rowPitch);
	AVPixelFormat ConvertDXGItoAV(DXGI_FORMAT fmt);
};

#endif	//__XVIDEO_WRITER_H__