/*Workaround libavformat bug with c++*/
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
//#include <windows.h>
extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}
//CLASS FOR VIDEO HANDLING 


class videoWriter
{
	private:
		AVOutputFormat *fmt2;
		AVFormatContext *oc;
		AVStream *video_st;
		AVCodecContext *pCodecCtx2;
		AVCodec *pCodec;
		AVCodecContext *pContext;
		//AVDictionary* avDictOpts;
		
		uint8_t *video_outbuf;
		uint8_t *picture_buf3;
		uint8_t *picture_buf2;
		uint8_t *rgb_buf;
		int video_outbuf_size;
		const char* filename;
		AVFrame *picture3;
		//AVPicture rgbPict;
		AVPixelFormat src_pix_fmt;
		AVPixelFormat formatIn;
		int uleveys, ukorkeus,framesEncoded;
		struct SwsContext *img_convert_ctx;
	public:
		
		//Function declarations
		void write_video_frame(uint8_t *imageIn[]);
		void write_trailer();
		videoWriter(const char* fname, int lev, int kor, AVPixelFormat src_pix_fmtIn, char* compression) //Constructor
		{
			    
				src_pix_fmt = src_pix_fmtIn;
				uleveys = lev;
				ukorkeus = kor;
				framesEncoded = 0;
				img_convert_ctx = NULL;
				filename = fname;
				printf("Tulosvideon nimi %s\n",filename);
				av_register_all();	//Rekisteröidään formaatit...
				//avcodec_register_all();	//Rekisteröidään koodekit
				printf("Registered_all\n");
				// allocate the output media context 
				avformat_alloc_output_context2(&oc,NULL, NULL, filename);
				if (!oc){
					printf("Couldn't alloc context\n");
					exit(1);
				}
				/*set output format*/
				fmt2 = oc->oformat;
				if (strcmp(compression,"x264")==0){
					fmt2->video_codec = AV_CODEC_ID_H264;//AV_CODEC_ID_MPEG1VIDEO;//AV_CODEC_ID_MJPEG;//
				}
				if (strcmp(compression,"mpeg1")==0){
					fmt2->video_codec = AV_CODEC_ID_MPEG1VIDEO;//AV_CODEC_ID_MPEG1VIDEO;//AV_CODEC_ID_MJPEG;//
				}
				/* add the video stream*/
				video_st = NULL;
				printf("video_codec %d\n",(int) fmt2->video_codec);
				 if (fmt2->video_codec != CODEC_ID_NONE) {
					/*GET X264codec*/
					pCodec =  avcodec_find_encoder(fmt2->video_codec);
					if (!pCodec) {
						fprintf(stderr, "Codec not found\n");
						exit(1);
					}
					printf("found encoder\n");
				 
					video_st = avformat_new_stream(oc, pCodec);
					if (!video_st) {
						fprintf(stderr, "Could not alloc stream\n");
						exit(1);
					}
					video_st->id = oc->nb_streams-1;
					pContext = video_st->codec;
					/* Adjust sample parameters */
					pContext->bit_rate = 1000000;
					/* resolution must be a multiple of two */
					pContext->width = uleveys;
					pContext->height = ukorkeus;
					/* frames per second */
					pContext->time_base= (AVRational){1,25};
					pContext->gop_size = 10; /* emit one intra frame every ten frames */
					pContext->max_b_frames=1;
					pContext->pix_fmt = AV_PIX_FMT_YUV420P;
					if (strcmp(compression,"x264")==0){
						pContext->profile = FF_PROFILE_H264_BASELINE;
					}
					pContext->flags |= AVFMT_ALLOW_FLUSH; //Try to allow flushing for x264
					printf("adjusted options\n");
					// some formats want stream headers to be separate
					if(oc->oformat->flags & AVFMT_GLOBALHEADER){
						video_st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
					}
					printf("Streami lisatty\n");
				}
				
				/* open codec */
				printf("Open codec\n");
				if (avcodec_open2(pContext, pCodec, NULL) < 0) {
					fprintf(stderr, "Could not open codec\n");
					exit(1);
				}
				printf("Opened codec %d\n",pContext->codec->id);	
				
				printf("Dump format\n");
				av_dump_format(oc, 0, filename, 1);
				/* open the output file */
				if (!(fmt2->flags & AVFMT_NOFILE)) {
					int ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
					if (ret < 0) {
						printf("Could not open '%s\n", filename);
						exit(1);
					}
				}
				printf("Header kirjoittamaan\n");	
				/* write the stream header, if any */
				avformat_write_header(oc,NULL);
				
				
				/*Create convert context*/
				img_convert_ctx = sws_getContext(uleveys, ukorkeus, src_pix_fmt,
                             uleveys, ukorkeus, pContext->pix_fmt,
                             SWS_BILINEAR, NULL, NULL, NULL);
				if (!img_convert_ctx){
					printf("Couldn't get convert context");
					exit(1);
				}
				pCodecCtx2 = video_st->codec;
				int size;


				picture3 = avcodec_alloc_frame();
				
				if (!picture3)
					printf("Ei kuvaa\n");
				size = avpicture_get_size(PIX_FMT_YUV420P, uleveys, ukorkeus);
				picture_buf3 = (uint8_t*) av_malloc(size);
				if (!picture_buf3) {
					av_free(picture3);
					printf("Ei bufferia\n");
				}
				avpicture_fill((AVPicture *)picture3, picture_buf3,
							   PIX_FMT_YUV420P, uleveys, ukorkeus);
				picture3->pts = 0;
				printf("Picture allocoitu, yeah\n");
				if (avpicture_alloc(&rgbPict,src_pix_fmt, uleveys, ukorkeus) <0){
					printf("Couldn't alloc\n");
					exit(0);				
				}
				/*
				printf("RGB pict alloc\n",size);
				size = avpicture_get_size(src_pix_fmt, uleveys, ukorkeus);
				printf("RGB pict size %d\n",size);
				rgb_buf = (uint8_t*) av_malloc(size);
				avpicture_fill(&rgbPict, rgb_buf,
							   src_pix_fmt, uleveys, ukorkeus);
				*/
				
		}
};

void videoWriter::write_video_frame(uint8_t* imageIn[])
{
	//printf("start writing frame");
	//avpicture_fill(&rgbPict,imageIn,src_pix_fmt,uleveys,ukorkeus);
	const int linesize[3] = {uleveys+16,uleveys+16,uleveys+16};
    int out_size, ret;
    AVCodecContext *c;
//	const int linesize[1] = {uleveys*3};

    c = video_st->codec;
	double pts = (double)video_st->pts.val * video_st->time_base.num /video_st->time_base.den;
	//printf("PTS calculated\n");
	if (formatIn != PIX_FMT_YUV420P) {
		//printf("Start Scaling\n");	   
		//sws_scale(img_convert_ctx, &rgbPict->data, &rgbPict->linesize, 0, c->height, picture3->data, picture3->linesize);
		sws_scale(img_convert_ctx, imageIn, linesize, 0, c->height, picture3->data, picture3->linesize);
		//printf("Scaled successfully\n");	   		
				

        } else {
			avpicture_fill((AVPicture *)picture3, *(imageIn), c->pix_fmt,
				c->width, c->height);
		}
    
	/*Try to show the image*/
	/*
	cimg_library::CImgDisplay pkuva1(c->width,c->height,"Pkuva1",0,false,true);
	pkuva1.move(0,20);
	cimg_library::CImg<unsigned char> testi(c->width,c->height,1,1,0);
	memcpy(testi,picture3->data[0],c->width*c->height*sizeof(unsigned char));
	pkuva1.display(testi);
	pkuva1.wait();
	*/
	/* encode the image */
	AVPacket pkt = { 0 };
	av_init_packet(&pkt);
	int got_output;
	out_size = avcodec_encode_video2(c, &pkt, picture3, &got_output);
	printf("oSize %d gPut %d\r",out_size,got_output);
	if (!out_size && got_output && pkt.size) {
		pkt.stream_index = video_st->index;

		/* Write the compressed frame to the media file. */
		ret = av_interleaved_write_frame(oc, &pkt);
		//printf("Wrote interleaved frame %d\n",ret);
	} else {
		ret = 0;
	}
	picture3->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
	++framesEncoded;

    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        exit(1);
    }

    
}

void videoWriter::write_trailer(){
	/*Get the delayed frames*/
    AVCodecContext *c;
    c = video_st->codec;
	int ret,out_size;
	// encode the image 
	int64_t pts = picture3->pts;
	
	int got_output;
	printf("\nFlushing\n");
	while (1){
		AVPacket pkt = { 0 };
	av_init_packet(&pkt);
	//for (int i = 0; i<10; ++i){
		out_size = avcodec_encode_video2(c, &pkt, NULL, &got_output);
		printf("Flushing oSize %d gPut %d\r",out_size,got_output);
		if (!out_size && got_output && pkt.size) {
			pkt.stream_index = video_st->index;
			pkt.pts = pts;
			pts+=av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
			// Write the compressed frame to the media file. 
			ret = av_interleaved_write_frame(oc, &pkt);
			//printf("Wrote interleaved frame %d\n",ret);
		} else {
			ret = 0;
			if (got_output < 1){
				break;
			}
		}
	}
	printf("Writing trailer");
	av_write_trailer(oc);
	printf("Closing file");
	avio_close(oc->pb);// Close the output file. 
	/*
    if (video_st){
		avcodec_close(video_st->codec);
		//av_free(picture3->data);
	}
    if (!(fmt2->flags & AVFMT_NOFILE)){
        avio_close(oc->pb);// Close the output file. 
	}
    // free the stream /
    avformat_free_context(oc);
	*/
}
