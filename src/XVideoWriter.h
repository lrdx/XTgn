#ifndef __XVIDEO_WRITER_H__
#define __XVIDEO_WRITER_H__

#include "logger.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
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
	XVideoWriter(Logger* logger);
	~XVideoWriter();

	bool IsInitialized() { return m_initialized; }

	void Initialize(const char* codec_name, const char* filename, AVPixelFormat fmt, int height, int width);
	void Release();
	void WriteFrame(uint8_t* buf, int rowCount, int rowPitch);
	void CloseFile();

private:
	Logger* m_logger;

	ffmpeg_context* m_video_context;
	
	bool m_initialized;

	void CopyBufferWithSws(uint8_t* buf, int rowCount, int rowPitch);
};

#endif	//__XVIDEO_WRITER_H__