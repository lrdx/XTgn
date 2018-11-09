#include "XVideoWriter.h"

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")

XVideoWriter::XVideoWriter(Logger* logger)
{
	m_initialized = false;
	m_logger = logger;
	m_video_context = new ffmpeg_context();
}

XVideoWriter::~XVideoWriter()
{
	Release();
	delete m_video_context;
}

void XVideoWriter::Initialize(const char* codec_name, const char* filename, AVPixelFormat fmt, int height, int width)
{
	if (m_initialized)
		Release();

	//Find encoder
	m_video_context->codec = avcodec_find_encoder_by_name(codec_name);
	if (!m_video_context->codec)
	{
		m_logger->write(QString("Codec %s not found. Using uncompressed video\r\n").arg(codec_name));
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
	m_video_context->ctx->bit_rate = 3500000;
	/* resolution must be a multiple of two */
	m_video_context->ctx->width = 800;
	m_video_context->ctx->height = 600;
	/* frames per second */
	const AVRational time_base = { 1, 30 };
	m_video_context->ctx->time_base = time_base;
	const AVRational framerate = { 30, 1 };
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
		char str_err[256];
		av_strerror(ret, str_err, 256);
		Release();
		m_logger->write(QString("Could not open codec: %s\r\n").arg(ret));
		return;
	}

	m_video_context->file = fopen(filename, "wb");
	if (!m_video_context->file) 
	{
		Release();
		m_logger->write(QString("Could not open file: %s\r\n").arg(filename));
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

	if (fmt != m_video_context->frame->format
		|| width != m_video_context->frame->width
		|| height != m_video_context->frame->height)
	{
		m_video_context->sws_ctx = sws_getContext(width, height, fmt,
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

		m_video_context->tmp_frame->format = fmt;
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

void XVideoWriter::Release()
{
	if(m_video_context->ctx)
		avcodec_free_context(&m_video_context->ctx);

	if(m_video_context->frame)
		av_frame_free(&m_video_context->frame);

	if (m_video_context->tmp_frame)
		av_frame_free(&m_video_context->tmp_frame);

	if(m_video_context->pkt)
		av_packet_free(&m_video_context->pkt);

	if(m_video_context->file)
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

void XVideoWriter::WriteFrame(uint8_t* buf, int rowCount, int rowPitch)
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
		CopyBufferWithSws(buf, rowCount, rowPitch);
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
	const uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	std::fflush(stdout);
	avcodec_send_frame(m_video_context->ctx, NULL);
	fwrite(endcode, 1, sizeof(endcode), m_video_context->file);
	fclose(m_video_context->file);

	Release();
}