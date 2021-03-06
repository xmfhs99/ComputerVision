#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "iplib.h"
#include "point.h"

/******************************************************************
	Name
		arith_add
	Function
		add gray-level of each pixel by a constant
	Input
		source : pointer to input image, unsigned char *
		number of pixels : # of pixels of input image
		val : addition constant
	Output
		source : input image is overwritten for output
	Remarks
******************************************************************/
void arith_add(unsigned char *source, int number_of_pixels, int val)
{
	int i, temp, LUT[256];

	/* initialize Look-up table */
	for (i = 0; i < 128; i++)
	{
		temp = i + val;
		CLIP(temp, 0, 255);
		LUT[i] = temp;
	}

	/* process image via the Look-up table */
	for (i = 0; i < number_of_pixels; i++)
		source[i] = LUT[source[i]];

}

void histogram_equalize(unsigned char *src, unsigned char *dst, int number_of_pixels) {
	unsigned long histogram[256];
	unsigned long sum_hist[256];
	float scale_factor;
	int sum, i;

	for (i = 0; i < 256; i++)
		histogram[i] = 0;	//히스토그램 값 0으로 초기화

	for (i = 0; i < number_of_pixels; i++)	//이후 source에 따라 히스토그램 값 설정
		histogram[src[i]]++;


	sum = 0;
	scale_factor = 255.0 / number_of_pixels;
	for (i = 0; i < 256; i++)
	{
		sum += histogram[i];
		sum_hist[i] = (int)((sum * scale_factor) + 0.5); //0.5 = 반올림
	}

	//sum_hist를 LUT로써 사용
	for (i = 0; i < number_of_pixels; i++)
		dst[i] = sum_hist[src[i]];	//출력 이미지 픽셀 설정
}

void auto_contrast_stretch(unsigned char *source, unsigned char *dst, int number_of_pixels, float low_percent, float high_percent) {
	int i, sum = 0;
	unsigned long histogram[256];
	unsigned char *LUT = (unsigned char*)malloc(256);
	int low = 0, high = 255;

	for (i = 0; i < 256; i++) //히스토그램 0으로 초기화
		histogram[i] = 0;
	for (i = 0; i < number_of_pixels; i++) //이후 source에 따른 히스토그램 값 설정
		histogram[source[i]]++;


	for (i = 0; i < 256; i++) { //하단부(low~, 0)부터 low_percent까지의 픽셀범위 계산
		sum += histogram[i];
		if (((sum * 100.0) / number_of_pixels) >= low_percent) { //퍼센트는 0이상 100이하의 퍼센트 실수값을 입력받으므로 100을 곱함
			low_percent = i; //low_percent값을 뛰어넘는 시점의 i로 값을 설정 후 break
			break;
		}
	}
	sum = 0;
	for (i = 255; i >= 0; i--) { //상단부(high~, 255)부터 high_percent까지의 픽셀범위를 계산하며, low와는 다르게 255부터 시작하여 감소
		sum += histogram[i];
		if (sum * 100 / number_of_pixels >= high_percent) { //low와 동일 방식
			high_percent = i;
			break;
		}
	}

	for (i = 0; i < low_percent; i++) { //설정된 값을 기반으로 일정 비율(low_percent)까지 0으로 값 맞춤
		LUT[i] = 0;
	}

	for (i = 255; i > high_percent; i--) { //마찬가지로 high_percent까지 255로 값 맞춤
		LUT[i] = 255;
	}


	//이후에 엔드인 공식을 이용해 low<x<high 범위 값을 설정
	for (i = low_percent; i <= high_percent; i++)
		LUT[i] = (unsigned long)(float)(255.0*(i - low_percent) / (high_percent - low_percent));

	for (int i = 0; i < number_of_pixels; i++)
		dst[i] = LUT[source[i]]; //출력 이미지 픽셀 설정

}



