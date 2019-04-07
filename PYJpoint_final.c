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

void binary_ellipse(unsigned char *src, unsigned char *dst, int number_of_pixels, int rows, int cols) {
	int **B = malloc(sizeof(int *) * rows);
	int i, cnt = 0, area = 0; //cnt = 1차원으로 저장된 기존 영상 src의 index
	int sumx = 0, sumy = 0; //중심 좌표 구할 때 사용할 값
	double disper20 = 0, disper02 = 0, disper11 = 0; //central moments 구할 때 사용할 값
	double centerx, centery; //중심 x,y 좌표
	double orient, r1, r2, elo; //r1 = 장축 r2 = 단축 elo = elongation orient = 방향

	for (i = 0; i < rows; i++) B[i] = malloc(sizeof(int) * cols); //1차원 영상 src => 2차원 이진 영상 B

	//2진 영상 2차원 배열 B 설정, 면적값 구하기
	for (i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (src[cnt] <= 60) {
				B[i][j] = 1; area++;
				sumx += j;
				sumy += i;
			}
			else B[i][j] = 0;
			cnt++;
		}
	}
	centerx = (double)sumx / area; //중심 x좌표
	centery = (double)sumy / area; //중심 y좌표

	//centerx,y 값을 이용하여 central moments 구하기 (disper20, disper02, disper11)
	for (i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (B[i][j] == 1) {
				disper20 += pow((j - centerx), 2.0);
				disper02 += pow((i - centery), 2.0);
				disper11 += (j - centerx) * (i - centery);
			}
		}
	}

	r1 = sqrt(2 * (disper20 + disper02 + sqrt(pow((disper20 - disper02), 2) + 4 * disper11*disper11))); //r1장축 공식
	r2 = sqrt(2 * (disper20 + disper02 - sqrt(pow((disper20 - disper02), 2) + 4 * disper11*disper11))); //r2단축 공식
	orient = atan2((disper02 - disper20), 2 * disper11) * (-0.5) * (180 / PI); //orientation
	elo = r1 / r2; //elongation

	printf("면적 : %d\n", area);
	printf("중심 좌표 : (%lf, %lf)\n", centerx, centery);
	printf("장축 r1 : %lf\n", r1);
	printf("단축 r2 : %lf\n", r2);
	printf("orientation : %lf\n", orient);
	printf("elongation : %lf\n\n", elo);

	//출력영상(dst) 처리, 원본영상에 중심좌표, 타원의 경계선을 흰 색(255)으로 표현
	for (i = 0; i < number_of_pixels; i++)
		dst[i] = src[i]; //src -> dst 복사

	dst[(cols*(sumy / area) - 1) + sumx / area] = 255; //중심좌표에 흰색 점 표시, 2차원 좌표를 1차원으로
	for (i = 0; i < rows; i++) { //타원 경계좌표 255로 설정
		for (int j = 0; j < cols; j++) {
			if (B[i][j] == 1) {
				if (B[i - 1][j] == 0 || B[i][j - 1] == 0 || B[i + 1][j] == 0 || B[i][j + 1] == 0)
					dst[i*cols + j] = 255;
			}
		}
	}

	/* //이진영상(B) 형태로 dst 처리, 이진영상에 중심좌표, 타원의 경계선을 회색(100)으로 표현
	for (i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			//dst에 이진영상 B 복사, threshhold = 60 기준 이하 = 255, 초과 = 0
			if(B[i][j] == 1)
				dst[i*cols + j] = 255;
			else dst[i*cols + j] = 0;
		}
	}

	dst[(cols*(sumy / area) - 1) + sumx / area] = 100; //중심좌표에 흰색 점 표시, 2차원 좌표를 1차원으로
	for (i = 0; i < rows; i++) { //타원 경계좌표 255로 설정
		for (int j = 0; j < cols; j++) {
			if (B[i][j] == 1) {
				if (B[i - 1][j] == 0 || B[i][j - 1] == 0 || B[i + 1][j] == 0 || B[i][j + 1] == 0)
					dst[i*cols + j] = 100;
			}
		}
	}*/
}