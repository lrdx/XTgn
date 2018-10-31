#ifndef __XVIDEO_WRITER_H__
#define __XVIDEO_WRITER_H__

#include "logger.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus 
}
#endif

struct ffmpeg_context
{
	AVCodec* codec;
	AVFrame* frame;
	AVCodecContext* ctx;
	AVPacket* pkt;
	FILE* file;
};

class XVideoWriter
{
public:
	XVideoWriter(Logger* logger);

	bool IsInitialized() { return m_initialized; }

	void Initialize(const char* codec_name, const char* filename);
	void Release();
	void WriteFrame();

private:
	Logger* m_logger;

	ffmpeg_context* m_video_context;

	bool m_initialized;
};

#endif	//__XVIDEO_WRITER_H__