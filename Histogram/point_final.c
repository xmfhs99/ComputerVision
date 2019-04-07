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
		histogram[i] = 0;	//������׷� �� 0���� �ʱ�ȭ

	for (i = 0; i < number_of_pixels; i++)	//���� source�� ���� ������׷� �� ����
		histogram[src[i]]++;


	sum = 0;
	scale_factor = 255.0 / number_of_pixels;
	for (i = 0; i < 256; i++)
	{
		sum += histogram[i];
		sum_hist[i] = (int)((sum * scale_factor) + 0.5); //0.5 = �ݿø�
	}

	//sum_hist�� LUT�ν� ���
	for (i = 0; i < number_of_pixels; i++)
		dst[i] = sum_hist[src[i]];	//��� �̹��� �ȼ� ����
}

void auto_contrast_stretch(unsigned char *source, unsigned char *dst, int number_of_pixels, float low_percent, float high_percent) {
	int i, sum = 0;
	unsigned long histogram[256];
	unsigned char *LUT = (unsigned char*)malloc(256);
	int low = 0, high = 255;

	for (i = 0; i < 256; i++) //������׷� 0���� �ʱ�ȭ
		histogram[i] = 0;
	for (i = 0; i < number_of_pixels; i++) //���� source�� ���� ������׷� �� ����
		histogram[source[i]]++;


	for (i = 0; i < 256; i++) { //�ϴܺ�(low~, 0)���� low_percent������ �ȼ����� ���
		sum += histogram[i];
		if (((sum * 100.0) / number_of_pixels) >= low_percent) { //�ۼ�Ʈ�� 0�̻� 100������ �ۼ�Ʈ �Ǽ����� �Է¹����Ƿ� 100�� ����
			low_percent = i; //low_percent���� �پ�Ѵ� ������ i�� ���� ���� �� break
			break;
		}
	}
	sum = 0;
	for (i = 255; i >= 0; i--) { //��ܺ�(high~, 255)���� high_percent������ �ȼ������� ����ϸ�, low�ʹ� �ٸ��� 255���� �����Ͽ� ����
		sum += histogram[i];
		if (sum * 100 / number_of_pixels >= high_percent) { //low�� ���� ���
			high_percent = i;
			break;
		}
	}

	for (i = 0; i < low_percent; i++) { //������ ���� ������� ���� ����(low_percent)���� 0���� �� ����
		LUT[i] = 0;
	}

	for (i = 255; i > high_percent; i--) { //���������� high_percent���� 255�� �� ����
		LUT[i] = 255;
	}


	//���Ŀ� ������ ������ �̿��� low<x<high ���� ���� ����
	for (i = low_percent; i <= high_percent; i++)
		LUT[i] = (unsigned long)(float)(255.0*(i - low_percent) / (high_percent - low_percent));

	for (int i = 0; i < number_of_pixels; i++)
		dst[i] = LUT[source[i]]; //��� �̹��� �ȼ� ����

}



