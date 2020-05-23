//图像结构体
struct SV_IMAGE_S
{
    SV_VOID* dataPtr;//数据域指针
    SV_S32 s32ImageType;//图像类型，可选值为SV_IMAGE_TYPE_UYVY，SV_IMAGE_TYPE_BGR，SV_IMAGE_TYPE_BGRA，SV_IMAGE_TYPE_YUV420P
    SV_SIZE_S stImageSize;
    SV_U32 u32Offset;
    SV_S32 s32BufIdx;
    SV_U64 u64Pts;
};


SV_S32 SV_ReSize(const SV_IMAGE_S& src, SV_IMAGE_S* dst, SV_U16* pSrcData);

#ifndef GET565RValue
#define GET565RValue(rgb)  (unsigned char)(((((unsigned short int)(rgb))&0xF800))>>8)
#endif
#ifndef GET565GValue
#define GET565GValue(rgb)  (unsigned char)(((((unsigned short int)(rgb))&0x07E0))>>3)
#endif
#ifndef GET565BValue
#define GET565BValue(rgb)  (unsigned char)(((((unsigned short int)(rgb))&0x001F))<<3)
#endif

#define CLIP(v) (unsigned char)((v)<0? 0 : (v>255?255:v))

#define GETY(r,g,b)  ( (  66 * (r) + 129 * (g) +  25 * (b) + 128) >> 8) +  16
#define GETU(r,g,b)  ( ( -38 * (r) -  74 * (g) + 112 * (b) + 128) >> 8) + 128
#define GETV(r,g,b)  ( ( 112 * (r) -  94 * (g) -  18 * (b) + 128) >> 8) + 128

//#define GETY(R,G,B)  ( 0.299 * R + 0.587 * G + 0.114 * B )
//#define GETU(R,G,B)  ( -0.1687 * R - 0.3313 * G + 0.5 * B + 128 )
//#define GETV(R,G,B)  ( 0.5 * R - 0.4187 * G - 0.0813 * B + 128 )

#define FB_COLOR_CONVERT_TEMP_BUF  	( 1920 * 1080 * 3 / 2 )
//#define FB_COLOR_CONVERT_BUF		( 720 * 480 * 3 / 2 )
#define FB_COLOR_CONVERT (720 * 480 * 2)

//SV_U8 buffYuv_1920_1080[FB_COLOR_CONVERT_TEMP_BUF] = {0};
//SV_U8 buffYuv_720_480[FB_COLOR_CONVERT_BUF] = {0};
SV_U16 buffRgb_720_480[FB_COLOR_CONVERT] = {0};

static SV_S32 InnerSV_RGB565ToYUV420P(SV_IMAGE_S& src,SV_IMAGE_S* pdst) {
	if(src.s32ImageType != SV_IMAGE_TYPE_UYVY && src.s32ImageType!=SV_IMAGE_TYPE_RGB565) {
		return SV_FAILURE;
	}
	if(src.s32ImageType == SV_IMAGE_TYPE_UYVY) {
	//	InnerSV_YUV422ToYUV420P(src,pdst);
		return SV_SUCCEED;
	}
	static char a = 0;
	if(a == 0){
		LOG(INFO) << "dW:" << pdst->stImageSize.s32Width << "dH:" << pdst->stImageSize.s32Height <<  "sW:" << src.stImageSize.s32Width << "sH:" << src.stImageSize.s32Height;
		a = 1;
	}
	/*一帧数据像素压缩*/
	if(SV_FAILURE==SV_ReSize(src, pdst, (SV_U16 *)buffRgb_720_480))
		return SV_FAILURE;

	SV_S32 s32W = pdst->stImageSize.s32Width;
	SV_S32 s32H = pdst->stImageSize.s32Height;
	const SV_S32 s32SrcW = src.stImageSize.s32Width;
	const SV_S32 s32SrcH = src.stImageSize.s32Height;
	SV_U16* ptrRGB565 = (SV_U16*)buffRgb_720_480;
//	memset(buffYuv_1920_1080, 0, FB_COLOR_CONVERT_TEMP_BUF);
	SV_U8* pY =(SV_U8*) pdst->dataPtr;
	SV_U8* pU =(SV_U8*)(pY+s32W*s32H);
	SV_U8* pV =(SV_U8*)(pU+(s32W*s32H>>2));
//	SV_U8* pY =	(SV_U8*)(buffYuv_1920_1080);	/*色彩转换临时缓存*/
//	SV_U8* pU = (SV_U8*)(pY+s32W*s32H);
//	SV_U8* pV = (SV_U8*)(pU+(s32W*s32H>>2));
	SV_U16 pWordBuf = 0;
	SV_U8 R,G,B;
	SV_U8 y, u, v;

	/*一帧数据色彩转换*/
	for(int row=0; row < s32H; row++) {		/*720*480偏移*/
		for(int col=0; col < s32W; col++) {	/*像素点偏移，两个字节*/
			static int l = 1;
			if(l) {
				LOG(INFO) << "img size equal,memcpy and quit!";
				l = 0;
			}
			//pWordBuf = *(ptrRGB565);
			memcpy(&pWordBuf,ptrRGB565,2);
			R = GET565RValue(pWordBuf);		/*获得一BYTE*/
			G = GET565GValue(pWordBuf);
			B = GET565BValue(pWordBuf);
			static int b = 1;
			if(b) {
				LOG(INFO) << "img size equal,memcpy and quit!";
				b = 0;
			}
			pWordBuf = 0xffff;
			y = CLIP(GETY(R,G,B));
			*pY++ = y;
			static int q = 1;
			if(q) {
				LOG(INFO) << "img size equal,memcpy and quit!";
				q = 0;
			}
			if( !(row%2) && !(col%2) ) {
				u = CLIP(GETU(R,G,B));
				*pU++ = u;
			}else if(!(col%2)) {
				v = CLIP(GETV(R,G,B));
				*pV++ = v;
			}
			ptrRGB565++;
		}
	}
	//memcpy(pdst->dataPtr, buffYuv_1920_1080, FB_COLOR_CONVERT_TEMP_BUF);
	return SV_SUCCEED;
}


SV_S32 SV_ReSize(const SV_IMAGE_S& src, SV_IMAGE_S* dst, SV_U16* pSrcData) {
	if(dst->stImageSize.s32Height == src.stImageSize.s32Height && dst->stImageSize.s32Width == src.stImageSize.s32Width) {
		memcpy(dst->dataPtr, pSrcData, FB_COLOR_CONVERT_TEMP_BUF);
		return SV_SUCCEED;
	}
	float srcWidth = (float)src.stImageSize.s32Width;
	float srcHeight = (float)src.stImageSize.s32Height;
	float dstWidth = (float)dst->stImageSize.s32Width;
	float dstHeight = (float)dst->stImageSize.s32Height;
	SV_U16* pDst_Y = pSrcData;	//(SV_U8*) dst->dataPtr;	//(SV_U8*) dst->dataPtr;
//	SV_U8* pDst_U =	(SV_U8*)(pDst_Y+(int)(dstWidth*dstHeight));
//	SV_U8* pDst_V =	(SV_U8*)(pDst_U+((int)(dstWidth*dstHeight)>>2)); //>>2
	SV_U16* pSrc_Y = (SV_U16*) src.dataPtr;
//	SV_U8* pSrc_U = (SV_U8*)(pSrc_Y+(int)(srcWidth*srcHeight));
//	SV_U8* pSrc_V = (SV_U8*)(pSrc_U+((int)(srcWidth*srcHeight)>>2)); //>>2
	/*YUV像素矩阵大小*/
	int src_w = 0;
	int src_h = 0;
	int dst_w = 0;
	int dst_h = 0;
	/*压缩比例*/
	float scale_x = (float)(srcWidth / dstWidth);
	float scale_y = (float)(srcHeight / dstHeight);
	SV_U16 *pSrc_YUV = NULL;
	SV_U16 *pDst_YUV = NULL;
//	 for(SV_U8 channal = 0; channal < 3; channal++) {
//	   if(channal == 0) {			/*通道Y*/
	     pSrc_YUV = pSrc_Y;
	     pDst_YUV = pDst_Y;
	     src_w = srcWidth;
	     src_h = srcHeight;
	     dst_w = dstWidth;
	     dst_h = dstHeight;
//	   }else if(channal == 1) {	/*通道U*/
//		 pSrc_YUV = pSrc_U;
//	     pDst_YUV = pDst_U;
//	     src_w = srcWidth / 2;
//	     src_h = srcHeight / 2;
//	     dst_w = dstWidth / 2;
//	     dst_h = dstHeight / 2;
//	   }else if(channal == 2) {	/*通道V*/
//	     pSrc_YUV = pSrc_V;
//	     pDst_YUV = pDst_V;
//	     src_w = srcWidth / 2;
//	     src_h = srcHeight / 2;
//	     dst_w = dstWidth / 2;
//	     dst_h = dstHeight / 2;
//	   }
	   for(int dst_y = 0; dst_y < dst_h; ++dst_y) {		/*对height遍历*/
	     for(int dst_x = 1; dst_x <= dst_w; ++dst_x) {
				/*目标在源上坐标*/
//				float src_x = (float)(((float)dst_x + 0.5) * scale_x - 0.5);
//				float src_y = (float)(((float)dst_y + 0.5) * scale_y - 0.5);
	         float src_x = (float)((float)(dst_x) * scale_x);		//--
	         float src_y = (float)((float)(dst_y) * scale_y);		//--
			/*计算在源图上四个近邻点的位置*/
//			int src_x_0 =(int)(src_x + 0.5);
//			int src_y_0 =(int)(src_y + 0.5);
//			int src_x_1 = ((src_x_0 + 1) < srcWidth)? src_x_0 + 1 : srcWidth;	//min(src_x_0 + 1, src_w - 1);
//			int src_y_1 = ((src_y_0 + 1) < srcHeight)? src_y_0 + 1 : srcHeight;	//min(src_y_0 + 1, src_h - 1);
//			/*双线性内插值*/
//			int value0 = (src_x_1 - src_x) * (SV_U16)(*(pSrc_YUV + (src_y_0 * (int)srcWidth + src_x_0)*2)) + \
//							(src_x - src_x_0) * (SV_U16)(*(pSrc_YUV + (src_y_0 * (int)srcWidth + src_x_1)*2));
//			int value1 = (src_x_1 - src_x) * (SV_U16)(*(pSrc_YUV + (src_y_1 * (int)srcWidth + src_x_0)*2)) + \
//							(src_x - src_x_0) * (SV_U16)(*(pSrc_YUV + (src_y_1 * (int)srcWidth + src_x_1)*2));
//			*(pDst_YUV + (dst_w * dst_y + dst_x)*2) = (SV_U16)((src_y_1 - src_y) * value0 + (src_y - src_y_0) * value1);
			/*最临近插值	srcX=dstX* (srcWidth/dstWidth) , srcY = dstY * (srcHeight/dstHeight)*/
			int src_x_ = (int)(src_x + 0.5);
			int src_y_ = (int)(src_y + 0.5);
			src_x_ = (src_x_ < srcWidth)? src_x_ : srcWidth ;	//min(src_x_0 + 1, src_w - 1);
			src_y_ = (src_y_ < srcHeight)? src_y_ : srcHeight ;	//min(src_y_0 + 1, src_h - 1);
			*(pDst_YUV + (dst_w * dst_y + dst_x)*2) = (SV_U16)(*(pSrc_YUV + (src_y_ * src_w + src_x_)*2));
			}
		}

//	}
//	memcpy(dst->dataPtr, buffYuv_720_480, FB_COLOR_CONVERT_BUF);	//FB_COLOR_CONVERT_BUF
	return SV_SUCCEED;
}