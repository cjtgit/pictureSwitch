#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
 
using namespace cv;

#define DST_WIDTH 720
#define DST_HEIGHT 480
#define C3C2 3/2
#define SV_U8 uchar

void doubleLineInsertVal(unsigned char *src, unsigned char*dst) {	
  float srcWidth = 1920;
  float srcHeight = 1080;
  float dstWidth = DST_WIDTH;
  float dstHeight = DST_HEIGHT; 
  printf("heighe is %d,width is %d\n",(int)dstHeight,(int)dstWidth); 
  if(src == NULL) {
    printf("src pointer is null\n");
  }
  if(dst == NULL) {
    printf("dst pointer is null\n");
  }
  SV_U8* pDst_Y = (SV_U8*) dst;
  SV_U8* pDst_U = (SV_U8*)(pDst_Y+(int)(dstWidth*dstHeight));
  SV_U8* pDst_V = (SV_U8*)(pDst_U+((int)(dstWidth*dstHeight)>>2));
  SV_U8* pSrc_Y = (SV_U8*)src;
  SV_U8* pSrc_U = (SV_U8*)(pSrc_Y+(int)(srcWidth*srcHeight));
  SV_U8* pSrc_V = (SV_U8*)(pSrc_U+((int)(srcWidth*srcHeight)>>2));

  SV_U8 *pSrc_YUV = NULL;
  SV_U8 *pDst_YUV = NULL;
  int src_w = 0;
  int src_h = 0;
  int dst_w = 0;
  int dst_h = 0;
 /*压缩比例*/
 float scale_x = (float)(srcWidth / dstWidth);
 float scale_y = (float)(srcHeight / dstHeight);

 for(SV_U8 channal = 0; channal < 3; channal++) {
   if(channal == 0) {			/*通道Y*/
     pSrc_YUV = pSrc_Y;
     pDst_YUV = pDst_Y;
     src_w = srcWidth;
     src_h = srcHeight;
     dst_w = dstWidth;
     dst_h = dstHeight;
   }else if(channal == 1) {	/*通道U*/
     pDst_YUV = pDst_U;
     pSrc_YUV = pSrc_U;
     src_w = srcWidth / 2;
     src_h = srcHeight / 2;
     dst_w = dstWidth / 2;
     dst_h = dstHeight / 2;
   }else if(channal == 2) {	/*通道V*/
     pSrc_YUV = pSrc_V;
     pDst_YUV = pDst_V;
     src_w = srcWidth / 2;
     src_h = srcHeight / 2;
     dst_w = dstWidth / 2;
     dst_h = dstHeight / 2;
   }
   for(int dst_y = 1; dst_y <= dst_h; ++dst_y) {		/*对height遍历*/
     for(int dst_x = 1; dst_x <= dst_w; ++dst_x) {
     /*目标在源上坐标*/
     //float src_x = ((dst_x + 0.5) * scale_x - 0.5);
     //float src_y = ((dst_y + 0.5) * scale_y - 0.5);
     float src_x = (float)((float)(dst_x) * scale_x);		//--
     float src_y = (float)((float)(dst_y) * scale_y);		//--
     /*计算在源图上四个近邻点的位置*/
     int src_x_0 =(int)(src_x + 0.5);
     int src_y_0 =(int)(src_y + 0.5);
     int src_x_1 = ((src_x_0 + 1) < (src_w - 1))? src_x_0 + 1 : src_w - 1;	//min(src_x_0 + 1, src_w - 1);
     int src_y_1 = ((src_y_0 + 1) < (src_h - 1))? src_y_0 + 1 : src_h - 1;	//min(src_y_0 + 1, src_h - 1);
     /*双线性内插值*/
     int value0 = (src_x_1 - src_x) * (SV_U8)(*(pSrc_YUV + src_y_0 * src_w + src_x_0)) + \
			(src_x - src_x_0) * (SV_U8)(*(pSrc_YUV + src_y_0 * src_w + src_x_1));
     int value1 = (src_x_1 - src_x) * (SV_U8)(*(pSrc_YUV + src_y_1 * src_w + src_x_0)) + \
			(src_x - src_x_0) * (SV_U8)(*(pSrc_YUV + src_y_1 * src_w + src_x_1));
     *pDst_YUV++ = (SV_U8)((src_y_1 - src_y) * value0 + (src_y - src_y_0) * value1);

	/*最临近插值	srcX=dstX* (srcWidth/dstWidth) , srcY = dstY * (srcHeight/dstHeight)*/
//	 int src_x_ = (int)(src_x + 0.5);		//--
//	 int src_y_ = (int)(src_y + 0.5);		//--
//   if(dst_y == dst_h && dst_x == dst_w) {
//		printf("the pic end is here!,src_x_ is %d ,src_y_ is %d\n",src_x_,src_y_);
//		printf("scale_x is %f ,scale_y is %f\n",scale_x,scale_y);

//	 }
//		src_x_ = (src_x_ < src_w)? src_x_ : src_w ;	//min(src_x_0 + 1, src_w - 1);
//		src_y_ = (src_y_ < src_h)? src_y_ : src_h ;	//min(src_y_0 + 1, src_h - 1);
//		*(pDst_YUV+(int)(dst_w*dst_y) + dst_x) = (SV_U8)(*(pSrc_YUV + src_y_ * src_w + src_x_));		//--
   }
 }
}
}


//降低图片采样
int reSize(unsigned char *src, unsigned char*dst) {
  doubleLineInsertVal(src, dst);
  return 0;
}



int main( int argc, char** argv )
{
    Mat cv_img;
    cv_img = imread(argv[1], 3);
    if( argc != 2 || !cv_img.data )
    {
        printf( "No image data \n" );
        return -1;
    }
	//rgb2yuv
    Mat cv_yuv;
    cvtColor(cv_img, cv_yuv, COLOR_BGR2YUV_I420);
    unsigned char *pFrame = (unsigned char *)malloc((cv_img.rows)*(cv_img.cols)*3/2);
    if(NULL == pFrame)
    {
     printf("malloc pFrame memory error\n");
    }
    memset(pFrame, 0,(cv_img.rows)*(cv_img.cols)*3/2);    
    memcpy(pFrame, cv_yuv.data, (cv_img.rows)*(cv_img.cols)*3/2);
 	//自定义降采样，降低图片分辨率为720*480
  	Mat cv_720_480;
  	cv_720_480.create(DST_HEIGHT*C3C2, DST_WIDTH , CV_8UC1);
  	reSize(pFrame, cv_720_480.data);
    //保存降采样后的yuv数据
    FILE* fp = fopen("./a.yuv", "wb+");
    if(NULL == fp)
    {
     printf("file open error\n");
     return -1;
    }
    fwrite(pFrame, 1, (cv_img.rows)*(cv_img.cols)*3/2, fp); 
	//yuv2jbp,保存jpg数据
	Mat cv_720_480jpg;
	cvtColor(cv_720_480, cv_720_480jpg, COLOR_YUV2BGR_I420);
	imwrite("b.jpg", cv_720_480jpg);
	imshow("720", cv_720_480jpg);
    namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
    imshow( "Display Image", cv_img );
    waitKey(0);
 
    return 0;

}
