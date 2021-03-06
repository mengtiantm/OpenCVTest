﻿#include "pch.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"



using namespace cv;
using namespace std;


unsigned char* ReadImageData(const char* fileName, int& iWidth, int& iHeight, int& iBitCount, int& iSize)
{
	unsigned char* uImageData = nullptr;
	FILE* fp = nullptr;
	fopen_s(&fp, fileName, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		iSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		uImageData = new unsigned char[iSize + 1];
		memset(uImageData, 0, iSize + 1);
		fread(uImageData, 1, iSize, fp);
		fclose(fp);
	}
	else
	{
		return nullptr;
	}

	unsigned char* uImageDataTemp = uImageData;
	BITMAPFILEHEADER fheader;
	memcpy(&fheader, uImageDataTemp, sizeof(BITMAPFILEHEADER));
	BITMAPINFOHEADER bmphdr;
	uImageDataTemp += sizeof(BITMAPFILEHEADER);
	memcpy(&bmphdr, uImageDataTemp, sizeof(BITMAPINFOHEADER));
	iWidth = bmphdr.biWidth;
	iHeight = bmphdr.biHeight;
	iBitCount = bmphdr.biBitCount;

	int iSizeNew = iSize - fheader.bfOffBits;
	unsigned char* uImageDataWithoutHeader = new unsigned char[iSizeNew + 1];
	memset(uImageDataWithoutHeader, 0, iSizeNew + 1);
	memcpy(uImageDataWithoutHeader, uImageData + fheader.bfOffBits, iSizeNew);
	
	delete[] uImageData;

	return uImageDataWithoutHeader;
}

void addImage(Mat image1, Mat image2, Mat& dst, double alpha, double beta, double gamma)
{
	if (image1.empty() || image2.empty())
	{
		return;
	}
	//将两幅图像分辨率调整一致
	if (image1.size() != image2.size())
	{
		resize(image1, image1, Size(image2.cols, image2.rows));
	}
	//融合操作
	addWeighted(image1, alpha, image2, beta, gamma, dst);
}

void addImageRoi(Mat image1, Mat image2, Mat& dst, double alpha, double beta, double gamma)
{
	if (image1.empty() || image2.empty())
	{
		return;
	}

	//判断大小
	if ((image1.cols < image2.cols) || (image1.rows < image2.rows))
	{
		return;
	}

	dst = image1.clone();
	Mat image_roi;
	//在dst上选定一个与要融合图像大小一样的区域,兴趣区域
	image_roi = dst(Rect(125, 125, image2.cols, image2.rows));
	//Mat image_roi(image1, Rect(125, 125, 256, 256));
	
	//将兴趣区域融合
	addWeighted(image_roi, alpha, image2, beta, gamma, dst);
}


/***********************************************************

** 函数名:GaussianFilter

** 输 入: uInputData,iWidth,iHeight,iBitCount,iTemplateSize

** uInputData---待处理图像数据

** iWidth -- 待处理图像宽度

** iHeight -- 待处理图像高度

** iBitCount -- 待处理图像位数

** iTemplateSize -- 滤波器模板大小（有3与5两种取值）

** 输 出: uOutputData --- 经过滤波处理后的图像数据

** 返回值：输出图像大小，大于0成功，小于等于0失败

** 功能描述:高斯低通滤波（图像平滑）

** 调用模块:OpenCV

** 作 者:TM

************************************************************/
unsigned int GaussianFilter(void* uInputData, int iWidth, int iHeight, int iBitCount, int iTemplateSize, void* uOutputData)
{
	unsigned int outputSize = 0;
	int iType = 0;
	if (iBitCount == 8)
	{
		iType = CV_8UC1;
		outputSize = iWidth * iHeight * 1;
	}
	else if (iBitCount == 24)
	{
		iType = CV_8UC3;
		outputSize = iWidth * iHeight * 3;
	}
	else if (iBitCount == 32)
	{
		iType = CV_8UC4;
		outputSize = iWidth * iHeight * 4;
	}

	Mat imgSrc(iHeight, iWidth, iType, uInputData);
	flip(imgSrc, imgSrc, 0);
	if (!imgSrc.data)
	{
		return 0;
	}
	
	Mat imgDest;
	if (iTemplateSize == 3)
	{
		GaussianBlur(imgSrc, imgDest, Size(3, 3), 0);
	}
	else if (iTemplateSize == 5)
	{
		GaussianBlur(imgSrc, imgDest, Size(5, 5), 0);
	}
	
	unsigned int retSize = 0;
	if (imgDest.data)
	{
		memcpy(uOutputData, imgDest.data, outputSize);
		retSize = outputSize;
	}
	return retSize;
}


/***********************************************************

** 函数名:LaplacianFilter

** 输 入: uInputData,iWidth,iHeight,iBitCount,iTemplateSize

** uInputData---待处理图像数据

** iWidth -- 待处理图像宽度

** iHeight -- 待处理图像高度

** iBitCount -- 待处理图像位数

** iTemplateSize -- 滤波器模板大小（有3与5两种取值）

** 输 出: uOutputData --- 经过滤波处理后的图像数据

** 返回值：输出图像大小，大于0成功，小于等于0失败

** 功能描述:拉普拉斯高通滤波（图像锐化）

** 调用模块:OpenCV

** 作 者:TM

************************************************************/
unsigned int LaplacianFilter(void* uInputData, int iWidth, int iHeight, int iBitCount, int iTemplateSize, void* uOutputData)
{
	unsigned int outputSize = 0;
	int iType = 0;
	if (iBitCount == 8)
	{
		iType = CV_8UC1;
		outputSize = iWidth * iHeight * 1;
	}
	else if (iBitCount == 24)
	{
		iType = CV_8UC3;
		outputSize = iWidth * iHeight * 3;
	}
	else if (iBitCount == 32)
	{
		iType = CV_8UC4;
		outputSize = iWidth * iHeight * 4;
	}

	Mat imgSrc(iHeight, iWidth, iType, uInputData);
	flip(imgSrc, imgSrc, 0);
	if (!imgSrc.data)
	{
		return 0;
	}

	int ddepth = imgSrc.depth();
	Mat imgDest;
	if (iTemplateSize == 3)
	{
		Mat kernel = (Mat_<float>(3, 3) << -1, -1, -1, -1, 8, -1, -1, -1, -1);
		filter2D(imgSrc, imgDest, ddepth, kernel);
	}
	else if (iTemplateSize == 5)
	{
		Mat kernel = (Mat_<float>(5, 5) << -2, -4, -4, -4, -2, -4, 0, 8, 0, -4, -4, 8, 24, 8, -4, -4, 0, 8, 0, -4, -2, -4, -4, -4, -2);
		filter2D(imgSrc, imgDest, ddepth, kernel);
	}

	unsigned int retSize = 0;
	if (imgDest.data)
	{
		memcpy(uOutputData, imgDest.data, outputSize);
		retSize = outputSize;
	}
	return retSize;
}
/***********************************************************

** 函数名:SharpeningFilter

** 输 入: uInputData,iWidth,iHeight,iBitCount,iTemplateSize，dAmount

** uInputData---待处理图像数据

** iWidth -- 待处理图像宽度

** iHeight -- 待处理图像高度

** iBitCount -- 待处理图像位数

** iTemplateSize -- 滤波器模板大小（有3与5两种取值）

** dAmount -- 锐化的程度（取值1-10，精度0.5，值越大，锐化效果越强）

** 输 出: uOutputData --- 经过滤波处理后的图像数据

** 返回值：输出图像大小，大于0成功，小于等于0失败

** 功能描述:锐化滤波（图像锐化）

** 调用模块:OpenCV

** 作 者:TM

************************************************************/
unsigned int SharpeningFilter(void* uInputData, int iWidth, int iHeight, int iBitCount, int iTemplateSize, double dAmount, void* uOutputData)
{
	unsigned int outputSize = 0;
	int iType = 0;
	if (iBitCount == 8)
	{
		iType = CV_8UC1;
		outputSize = iWidth * iHeight * 1;
	}
	else if (iBitCount == 24)
	{
		iType = CV_8UC3;
		outputSize = iWidth * iHeight * 3;
	}
	else if (iBitCount == 32)
	{
		iType = CV_8UC4;
		outputSize = iWidth * iHeight * 4;
	}

	Mat imgSrc(iHeight, iWidth, iType, uInputData);
	flip(imgSrc, imgSrc, 0);
	if (!imgSrc.data)
	{
		return 0;
	}

	Mat imgblur;
	if (iTemplateSize == 3)
	{
		GaussianBlur(imgSrc, imgblur, Size(3, 3), 0);
	}
	else if (iTemplateSize == 5)
	{
		GaussianBlur(imgSrc, imgblur, Size(5, 5), 0);
	}

	Mat imgDest;
	addWeighted(imgSrc, dAmount, imgblur, (1 - dAmount), 0, imgDest);

	unsigned int retSize = 0;
	if (imgDest.data)
	{
		memcpy(uOutputData, imgDest.data, outputSize);
		retSize = outputSize;
	}
	return retSize;
}


/***********************************************************

** 函数名:ImagePseudoColor

** 输 入: uInputData,iWidth,iHeight,iBitCount,uColorMap

** uInputData---待处理图像数据

** iWidth -- 待处理图像宽度

** iHeight -- 待处理图像高度

** iBitCount -- 待处理图像位数

** uColorMap -- 伪彩表数据（256*3字节RGB数据）

** 输 出: uOutputData --- 经过伪彩映射处理后的图像数据

** 返回值：输出图像大小，大于0成功，小于等于0失败

** 功能描述:图像伪彩映射——将灰度图像映射成彩色图像

** 调用模块:OpenCV

** 作 者:TM

************************************************************/
unsigned int ImagePseudoColor(void* uInputData, int iWidth, int iHeight, int iBitCount, unsigned char* uColorMap, void* uOutputData)
{
	unsigned int outputSize = iWidth * iHeight * 3;
	int iType = 0;
	int iCode = CV_RGBA2RGB;
	int iCodeReverse = CV_RGB2RGBA;
	if (iBitCount == 8)
	{
		iType = CV_8UC1;
		iCode = CV_GRAY2RGB;
	}
	else if (iBitCount == 24)
	{
		iType = CV_8UC3;
	}
	else if (iBitCount == 32)
	{
		iType = CV_8UC4;
		iCode = CV_RGBA2RGB;
		iCodeReverse = CV_RGB2RGBA;
		outputSize = iWidth * iHeight * 4;
	}

	Mat imgSrc(iHeight, iWidth, iType, uInputData);
	flip(imgSrc, imgSrc, 0);
	if (imgSrc.channels() != 3)
		cvtColor(imgSrc, imgSrc, iCode);

	if (!imgSrc.data)
	{
		return 0;
	}

	Mat imgDest;
	Mat lut_table(1, 256, CV_8UC3);
	for (int i = 0; i < 256; ++i)
	{
		lut_table.at<cv::Vec3b>(0, i)[2] = uColorMap[i];
		lut_table.at<cv::Vec3b>(0, i)[1] = uColorMap[i + 256];
		lut_table.at<cv::Vec3b>(0, i)[0] = uColorMap[i + 256 * 2];
	}
	LUT(imgSrc, lut_table, imgDest);
	
	if (iBitCount == 32)
	{
		cvtColor(imgDest, imgDest, iCodeReverse);
	}

	unsigned int retSize = 0;
	if (imgDest.data)
	{
		memcpy(uOutputData, imgDest.data, outputSize);
		retSize = outputSize;
	}
	return retSize;
}


/***********************************************************

** 函数名:ImageSubtract

** 输 入: srcImage,maskImage,iWidth,iHeight,iBitCount,offsetX,offsetY

** srcImage---原图像数据

** maskImage -- 掩码图像数据

** iWidth -- 原图与掩码图宽度

** iHeight -- 原图与掩码图高度

** iBitCount -- 原图与掩码图位数

** offsetX -- 偏移量X

** offsetY -- 偏移量Y

** 输 出: uOutputData --- 经过减影处理后的图像数据

** 返回值：输出图像大小，大于0减影成功，小于等于0失败

** 功能描述:图像减影

** 调用模块:OpenCV

** 作 者:TM

************************************************************/
int ImageSubtract(void* srcImage, void* maskImage, int iWidth, int iHeight, int iBitCount, int offsetX, int offsetY, void* uOutputData)
{
	int outputSize = iWidth * iHeight * 3;
	int iType = 0;
	int iCode = CV_RGB2GRAY;
	int iCodeReverse = CV_GRAY2RGB;
	if (iBitCount == 8)
	{
		iType = CV_8UC1;
		outputSize = iWidth * iHeight * 1;
	}
	else if (iBitCount == 24)
	{
		iType = CV_8UC3;
		iCode = CV_RGB2GRAY;
		iCodeReverse = CV_GRAY2RGB;
	}
	else if (iBitCount == 32)
	{
		iType = CV_8UC4;
		iCode = CV_RGBA2GRAY;
		iCodeReverse = CV_GRAY2RGBA;
		outputSize = iWidth * iHeight * 4;
	}

	Mat imgSrc(iHeight, iWidth, iType, srcImage);
	flip(imgSrc, imgSrc, 0);
	Mat imgMask(iHeight, iWidth, iType, maskImage);
	flip(imgMask, imgMask, 0);
	if (!imgSrc.data || !imgMask.data)
	{
		return 0;
	}
	Mat imgSrcGray, imgMaskGray;
	if (iBitCount == 8)
	{
		imgSrcGray = imgSrc;
		imgMaskGray = imgMask;
	}
	else
	{
		cvtColor(imgSrc, imgSrcGray, iCode);
		cvtColor(imgMask, imgMaskGray, iCode);
	}

	auto a = imgSrcGray;
	auto b = imgMaskGray;
	auto mask = b.clone();
	if (offsetX != 0 || offsetY != 0)
	{
		mask.setTo(0);
		auto offsetX_ = abs(offsetX);
		auto offsetY_ = abs(offsetY);
		if (offsetX > 0 && offsetY > 0)
		{
			auto mm = mask(Range(offsetY_, b.rows), Range(offsetX_, b.cols));
			auto bb = b(Range(0, b.rows - offsetY_), Range(0, b.cols - offsetX_));
			bb.copyTo(mm);
		}
		else
		{
			auto mm = b(Range(offsetY_, b.rows), Range(offsetX_, b.cols));
			auto bb = mask(Range(0, b.rows - offsetY_), Range(0, b.cols - offsetX_));
			mm.copyTo(bb);
		}
	}

	Mat subtract1, subtract2, blank;
	subtract(a, mask, subtract1);
	auto es = subtract1.elemSize();
	auto maxValue = max_element(subtract1.begin<unsigned char>(), subtract1.end<unsigned char>());
	auto scale = 256 / (*maxValue);

	convertScaleAbs(subtract1, subtract1, scale, 0);

	blank.create(subtract1.rows, subtract1.cols, subtract1.type());
	blank.setTo(255);
	subtract(blank, subtract1, subtract2);

	convertScaleAbs(subtract2, blank, 1, -127);

	if (iBitCount != 8)
	{
		cvtColor(blank, blank, iCodeReverse);
	}
	unsigned int retSize = 0;
	if (blank.data)
	{
		retSize = outputSize;
		memcpy(uOutputData, blank.data, outputSize);
	}
	return retSize;
}


int main()
{
	string strFileName = "original.bmp";
	Mat imgSrc = imread(strFileName, IMREAD_UNCHANGED);
	namedWindow("Original", 0);
	//cvResizeWindow("Original", 640, 640);
	imshow("Original", imgSrc);
	
	int iWidth = 0, iHeight = 0, iBitCount = 8, iSize = 0;
	unsigned char* uInImageData = ReadImageData(strFileName.c_str(), iWidth, iHeight, iBitCount, iSize);
	
	int iType = 0;
	if (iBitCount == 8)
	{
		iType = CV_8UC1;
	}
	else if (iBitCount == 24)
	{
		iType = CV_8UC3;
	}
	else if (iBitCount == 32)
	{
		iType = CV_8UC4;
	}
	
	unsigned int uRetSize = 0;
	unsigned char* uOutImageData = new unsigned char[iSize + 1];
	memset(uOutImageData, 0, iSize + 1);
	/***********************图像平滑**************************/
	/*uRetSize = GaussianFilter(uInImageData, iWidth, iHeight, iBitCount, 3, uOutImageData);
	Mat imgGaussian3(iHeight, iWidth, iType, uOutImageData);
	imshow("Gaussian3", imgGaussian3);*/
	

	/***********************图像锐化**************************/
	////uRetSize = LaplacianFilter(uInImageData, iWidth, iHeight, iBitCount, 3, uOutImageData);
	//uRetSize = LaplacianFilter(uInImageData, iWidth, iHeight, iBitCount, 5, uOutImageData);
	//Mat imgLaplacian3(iHeight, iWidth, iType, uOutImageData);
	//imshow("Laplacian3", imgLaplacian3);
	
	//uRetSize = SharpeningFilter(uInImageData, iWidth, iHeight, iBitCount, 3, 2.5, uOutImageData);
	uRetSize = SharpeningFilter(uInImageData, iWidth, iHeight, iBitCount, 3, 10, uOutImageData);
	Mat imgSharpening3(iHeight, iWidth, iType, uOutImageData);
	imshow("Sharpening3", imgSharpening3);


	/***********************图像伪彩**************************/
	/*FILE* fp1 = nullptr;
	uchar uszPseudo[256 * 3] = { 0 };
	fopen_s(&fp1, "rainbow.lut", "rb");
	if (fp1)
	{
		fread(uszPseudo, 1, 768, fp1);
		fclose(fp1);
	}
	if (imgSrc.channels() == 1)
	{
		iSize = iWidth * iHeight * 3;
		uOutImageData = new unsigned char[iSize + 1];
		memset(uOutImageData, 0, iSize + 1);
		iType = CV_8UC3;
	}
	uRetSize = ImagePseudoColor(uInImageData, iWidth, iHeight, iBitCount, uszPseudo, uOutImageData);
	Mat imgPseudoColor(iHeight, iWidth, iType, uOutImageData);
	imshow("imgPseudoColor", imgPseudoColor);*/


	/***********************图像减影**************************/
	/*unsigned char* pszImageDataSrc = ReadImageData("sourceImage.bmp", iWidth, iHeight, iBitCount, iSize);
	unsigned char* pszImageDataMask = ReadImageData("maskImage.bmp", iWidth, iHeight, iBitCount, iSize);
	unsigned char* uOutImageData1 = new unsigned char[iSize];
	memset(uOutImageData1, 0, iSize);
	uRetSize = ImageSubtract(pszImageDataSrc, pszImageDataMask, iWidth, iHeight, iBitCount, 0, 0, uOutImageData1);
	Mat imgSubtract(iWidth, iHeight, CV_8UC4, uOutImageData1);
	imshow("Subtract", imgSubtract);
	delete[] uOutImageData1;*/

	
	/***********************图像融合**************************/
	//double alpha = 0.5;
	//double beta = 0.5;
	//double gamma = 0.0;
	//Mat imageDst;
	////读取两幅影像
	//Mat image1 = imread("CT.jpg");
	//Mat image2 = imread("PET.jpg");

	////图像融合有两种方式，第一种是全部融合，要求输入图像大小是一样的。
	//addImage(image1, image2, imageDst, alpha, beta, gamma);
	////第二种是感兴趣（ROI）区域融会。
	////addImageRoi(image1, image2, imageDst, alpha, beta, gamma);

	////显示结果
	//namedWindow("fusion", 0);
	//imshow("fusion", imageDst);

	if (uInImageData)
	{
		delete[] uInImageData;
	}
	if (uOutImageData)
	{
		delete[] uOutImageData;
	}
	waitKey();
	

	return 0;
}
