#include "XVideoWriter.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavdevice/avdevice.h>
#ifdef __cplusplus 
}
#endif

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")

XVideoWriter::XVideoWriter(Logger* logger)
	: m_logger(logger), m_initialized(false)
{
	m_video_context = std::make_unique<ffmpeg_context>();
}

XVideoWriter::~XVideoWriter()
{
	Release();
}

void XVideoWriter::Release()
{
	if (m_video_context->ctx)
		avcodec_free_context(&m_video_context->ctx);

	if (m_video_context->frame)
		av_frame_free(&m_video_context->frame);

	if (m_video_context->tmp_frame)
		av_frame_free(&m_video_context->tmp_frame);

	if (m_video_context->pkt)
		av_packet_free(&m_video_context->pkt);

	if (m_video_context->file)
		fclose(m_video_context->file);

	if (m_video_context->sws_ctx)
		sws_freeContext(m_video_context->sws_ctx);

	m_video_context->ctx = nullptr;
	m_video_context->frame = nullptr;
	m_video_context->tmp_frame = nullptr;
	m_video_context->pkt = nullptr;
	m_video_context->file = nullptr;
	m_video_context->sws_ctx = nullptr;

	m_initialized = false;
}

void XVideoWriter::Initialize(const std::string& filename,
	const std::string& codecName,
	int videoBitrate,
	int videoWidth,
	int videoHeight,
	int videoFramerate,
	DXGI_FORMAT format,
	int height,
	int width)
{
	Initialize(filename, codecName, videoBitrate, videoWidth, videoHeight, videoFramerate, ConvertDXGItoAV(format), height, width);
}

void XVideoWriter::Initialize(const std::string& filename,
	const std::string& codecName,
	int videoBitrate,
	int videoWidth,
	int videoHeight,
	int videoFramerate,
	AVPixelFormat format,
	int height,
	int width)
{
	if(format == AV_PIX_FMT_NONE)
	{
		m_logger->write("Screen format unknown");
		return;
	}

	if (m_initialized)
		Release();

	//Find encoder
	m_video_context->codec = avcodec_find_encoder_by_name(codecName.c_str());
	if (!m_video_context->codec)
	{
		m_logger->write(QString("Codec %s not found. Using uncompressed video\r\n").arg(codecName.c_str()));
	}

	m_video_context->ctx = avcodec_alloc_context3(m_video_context->codec);
	if (!m_video_context->ctx)
	{
		Release();
		m_logger->write("Could not allocate video codec context\r\n");
		return;
	}

	m_video_context->pkt = av_packet_alloc();
	if (!m_video_context->pkt)
	{
		Release();
		m_logger->write("Could not allocate video packet\r\n");
		return;
	}

	/* put sample parameters */
	m_video_context->ctx->bit_rate = videoBitrate;
	/* resolution must be a multiple of two */
	m_video_context->ctx->width = videoWidth;
	m_video_context->ctx->height = videoHeight;
	/* frames per second */
	const AVRational tb = { 1, videoFramerate };
	m_video_context->ctx->time_base = tb;
	const AVRational fr = { videoFramerate, 1 };
	m_video_context->ctx->framerate = fr;

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
	auto ret = avcodec_open2(m_video_context->ctx, m_video_context->codec, nullptr);
	if (ret < 0)
	{
		char str_err[256];
		av_strerror(ret, str_err, 256);
		Release();
		m_logger->write(QString("Could not open codec: %s\r\n").arg(ret));
		return;
	}

	m_video_context->file = fopen(filename.c_str(), "wb");
	if (!m_video_context->file) 
	{
		Release();
		m_logger->write(QString("Could not open file: %s\r\n").arg(filename.c_str()));
		return;
	}

	m_video_context->frame = av_frame_alloc();
	if (!m_video_context->frame) 
	{
		Release();
		m_logger->write("Could not allocate video frame\r\n");
		return;
	}

	m_video_context->frame->format = m_video_context->ctx->pix_fmt;
	m_video_context->frame->width = m_video_context->ctx->width;
	m_video_context->frame->height = m_video_context->ctx->height;

	ret = av_frame_get_buffer(m_video_context->frame, 0);
	if (ret < 0)
	{
		Release();
		m_logger->write("Could not allocate the video frame data\r\n");
		return;
	}

	if (format != m_video_context->frame->format
		|| width != m_video_context->frame->width
		|| height != m_video_context->frame->height)
	{
		m_video_context->sws_ctx = sws_getContext(width, height, format,
			m_video_context->frame->width, m_video_context->frame->height,
			static_cast<AVPixelFormat>(m_video_context->frame->format), SWS_BICUBIC, NULL, NULL, NULL);

		if (!m_video_context->sws_ctx)
		{
			m_logger->write("Could not allocate the sws context\r\n");
			Release();
			return;
		}

		m_video_context->tmp_frame = av_frame_alloc();
		if (!m_video_context->tmp_frame)
		{
			Release();
			m_logger->write("Could not allocate tmp video frame\r\n");
			return;
		}

		m_video_context->tmp_frame->format = format;
		m_video_context->tmp_frame->width = width;
		m_video_context->tmp_frame->height = height;

		ret = av_frame_get_buffer(m_video_context->tmp_frame, 0);
		if (ret < 0)
		{
			Release();
			m_logger->write("Could not allocate the tmp video frame data\r\n");
			return;
		}
	}

	m_video_context->frame_pts = 0;
	m_initialized = true;
}

void XVideoWriter::CopyBufferWithSws(uint8_t* buf, int rowCount, int rowPitch)
{
	uint8_t* sptr = buf;
	uint8_t* dptr = m_video_context->tmp_frame->data[0];

	for (size_t h = 0; h < rowCount; ++h)
	{
		memcpy(dptr, sptr, rowPitch);
		sptr += rowPitch;
		dptr += m_video_context->tmp_frame->linesize[0];
	}

	sws_scale(m_video_context->sws_ctx,
		(const uint8_t * const *)m_video_context->tmp_frame->data,
		m_video_context->tmp_frame->linesize, 0, m_video_context->tmp_frame->height, m_video_context->frame->data,
		m_video_context->frame->linesize);
}

AVPixelFormat XVideoWriter::ConvertDXGItoAV(const DXGI_FORMAT fmt)
{
	switch(fmt)
	{
	case DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT:
		return AVPixelFormat::AV_PIX_FMT_RGBA;
	default:
		return AVPixelFormat::AV_PIX_FMT_NONE;
	}
}

void XVideoWriter::WriteFrame(uint8_t* buf, int row_count, int row_pitch)
{
	if (!m_initialized)
		return;

	if (av_frame_make_writable(m_video_context->frame) < 0)
	{
		m_logger->write("Error write frame to file: frame unwritable");
		Release();
	}

	if (m_video_context->sws_ctx)
	{
		CopyBufferWithSws(buf, row_count, row_pitch);
	}

	m_video_context->frame->pts = m_video_context->frame_pts++;

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

void XVideoWriter::CloseFile()
{
	if (!m_initialized)
		return;

	const uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	std::fflush(stdout);
	avcodec_send_frame(m_video_context->ctx, nullptr);
	fwrite(endcode, 1, sizeof(endcode), m_video_context->file);
	fclose(m_video_context->file);

	Release();
}



std::vector<std::string> XVideoWriter::GetAllEncoders()
{
	std::vector<std::string> vec;

	void* opaque = nullptr;
	auto codec = av_codec_iterate(&opaque);
	while(codec)
	{
		if (av_codec_is_encoder(codec))
			vec.emplace_back(codec->name);

		codec = av_codec_iterate(&opaque);
	}

	return vec;
}