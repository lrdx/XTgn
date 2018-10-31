#include "XVideoWriter.h"

#pragma comment(lib, "ffmpeg/lib/avutil.lib")
#pragma comment(lib, "ffmpeg/lib/avcodec.lib")

XVideoWriter::XVideoWriter(Logger* logger)
{
	m_initialized = false;

	m_video_context = new ffmpeg_context();

	m_logger = logger;
}

void XVideoWriter::Initialize(const char* codec_name, const char* filename)
{
	//Find encoder
	m_video_context->codec = avcodec_find_encoder_by_name(codec_name);
	if (!m_video_context->codec)
	{
		m_logger->write(QString("Codec %s not found... using uncompressed video\r\n").arg(codec_name));
	}

	m_video_context->ctx = avcodec_alloc_context3(m_video_context->codec);
	if (!m_video_context->ctx) 
	{
		m_logger->write("Could not allocate video codec context\r\n");
		return;
	}

	m_video_context->pkt = av_packet_alloc();
	if (!m_video_context->pkt)
	{
		m_logger->write("Could not allocate video packet\r\n");
		Release();
		return;
	}

	/* put sample parameters */
	m_video_context->ctx->bit_rate = 400000;
	/* resolution must be a multiple of two */
	m_video_context->ctx->width = 352;
	m_video_context->ctx->height = 288;
	/* frames per second */
	const AVRational time_base = { 1, 25 };
	m_video_context->ctx->time_base = time_base;
	const AVRational framerate = { 25, 1 };
	m_video_context->ctx->framerate = framerate;

	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	m_video_context->ctx->gop_size = 10;
	m_video_context->ctx->max_b_frames = 1;
	m_video_context->ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (m_video_context->codec->id == AV_CODEC_ID_H264)
		av_opt_set(m_video_context->ctx->priv_data, "preset", "slow", 0);

	/* open it */
	auto ret = avcodec_open2(m_video_context->ctx, m_video_context->codec, NULL);
	if (ret < 0)
	{
		m_logger->write(QString("Could not open codec: %s\r\n").arg(ret));
		Release();
		return;
	}

	m_video_context->file = fopen(filename, "wb");
	if (!m_video_context->file) 
	{
		m_logger->write(QString("Could not open file: %s\r\n").arg(filename));
		Release();
		return;
	}

	m_video_context->frame = av_frame_alloc();
	if (!m_video_context->frame) 
	{
		m_logger->write("Could not allocate video frame\r\n");
		Release();
		return;
	}

	m_video_context->frame->format = m_video_context->ctx->pix_fmt;
	m_video_context->frame->width = m_video_context->ctx->width;
	m_video_context->frame->height = m_video_context->ctx->height;

	ret = av_frame_get_buffer(m_video_context->frame, 0);
	if (ret < 0)
	{
		m_logger->write("Could not allocate the video frame data\r\n");
		Release();
		return;
	}

	m_initialized = true;
}

void XVideoWriter::Release()
{
	if(m_video_context->ctx)
		avcodec_free_context(&m_video_context->ctx);

	if(m_video_context->frame)
		av_frame_free(&m_video_context->frame);

	if(m_video_context->pkt)
		av_packet_free(&m_video_context->pkt);

	if(m_video_context->file)
		fclose(m_video_context->file);

	m_video_context->ctx = nullptr;
	m_video_context->frame = nullptr;
	m_video_context->pkt = nullptr;
	m_video_context->file = nullptr;

	m_initialized = false;
}

bool XVideoWriter::GetFrameBuffer(uint8_t* buffer)
{
	if (!m_initialized)
		return false;

	const auto ret = av_frame_make_writable(m_video_context->frame);
	if (ret < 0)
	{
		m_logger->write("Could not make writable frame");
		return false;
	}

	buffer = *m_video_context->frame->data;
	return true;
}

void XVideoWriter::WriteFrame()
{
	if (!m_initialized)
		return;

	auto ret = avcodec_send_frame(m_video_context->ctx, m_video_context->frame);
	if (ret < 0)
	{
		m_logger->write("Error sending a frame for encoding\r\n");
		return;
	}

	while (ret >= 0)
	{
		ret = avcodec_receive_packet(m_video_context->ctx, m_video_context->pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0)
		{
			m_logger->write("Error during encoding\r\n");
			return;
		}

		fwrite(m_video_context->pkt->data, 1, m_video_context->pkt->size, m_video_context->file);
		av_packet_unref(m_video_context->pkt);
	}
}