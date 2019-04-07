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

void binary_ellipse(unsigned char *src, unsigned char *dst, int number_of_pixels, int rows, int cols) {
	int **B = malloc(sizeof(int *) * rows);
	int i, cnt = 0, area = 0; //cnt = 1�������� ����� ���� ���� src�� index
	int sumx = 0, sumy = 0; //�߽� ��ǥ ���� �� ����� ��
	double disper20 = 0, disper02 = 0, disper11 = 0; //central moments ���� �� ����� ��
	double centerx, centery; //�߽� x,y ��ǥ
	double orient, r1, r2, elo; //r1 = ���� r2 = ���� elo = elongation orient = ����

	for (i = 0; i < rows; i++) B[i] = malloc(sizeof(int) * cols); //1���� ���� src => 2���� ���� ���� B

	//2�� ���� 2���� �迭 B ����, ������ ���ϱ�
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
	centerx = (double)sumx / area; //�߽� x��ǥ
	centery = (double)sumy / area; //�߽� y��ǥ

	//centerx,y ���� �̿��Ͽ� central moments ���ϱ� (disper20, disper02, disper11)
	for (i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (B[i][j] == 1) {
				disper20 += pow((j - centerx), 2.0);
				disper02 += pow((i - centery), 2.0);
				disper11 += (j - centerx) * (i - centery);
			}
		}
	}

	r1 = sqrt(2 * (disper20 + disper02 + sqrt(pow((disper20 - disper02), 2) + 4 * disper11*disper11))); //r1���� ����
	r2 = sqrt(2 * (disper20 + disper02 - sqrt(pow((disper20 - disper02), 2) + 4 * disper11*disper11))); //r2���� ����
	orient = atan2((disper02 - disper20), 2 * disper11) * (-0.5) * (180 / PI); //orientation
	elo = r1 / r2; //elongation

	printf("���� : %d\n", area);
	printf("�߽� ��ǥ : (%lf, %lf)\n", centerx, centery);
	printf("���� r1 : %lf\n", r1);
	printf("���� r2 : %lf\n", r2);
	printf("orientation : %lf\n", orient);
	printf("elongation : %lf\n\n", elo);

	//��¿���(dst) ó��, �������� �߽���ǥ, Ÿ���� ��輱�� �� ��(255)���� ǥ��
	for (i = 0; i < number_of_pixels; i++)
		dst[i] = src[i]; //src -> dst ����

	dst[(cols*(sumy / area) - 1) + sumx / area] = 255; //�߽���ǥ�� ��� �� ǥ��, 2���� ��ǥ�� 1��������
	for (i = 0; i < rows; i++) { //Ÿ�� �����ǥ 255�� ����
		for (int j = 0; j < cols; j++) {
			if (B[i][j] == 1) {
				if (B[i - 1][j] == 0 || B[i][j - 1] == 0 || B[i + 1][j] == 0 || B[i][j + 1] == 0)
					dst[i*cols + j] = 255;
			}
		}
	}

	/* //��������(B) ���·� dst ó��, �������� �߽���ǥ, Ÿ���� ��輱�� ȸ��(100)���� ǥ��
	for (i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			//dst�� �������� B ����, threshhold = 60 ���� ���� = 255, �ʰ� = 0
			if(B[i][j] == 1)
				dst[i*cols + j] = 255;
			else dst[i*cols + j] = 0;
		}
	}

	dst[(cols*(sumy / area) - 1) + sumx / area] = 100; //�߽���ǥ�� ��� �� ǥ��, 2���� ��ǥ�� 1��������
	for (i = 0; i < rows; i++) { //Ÿ�� �����ǥ 255�� ����
		for (int j = 0; j < cols; j++) {
			if (B[i][j] == 1) {
				if (B[i - 1][j] == 0 || B[i][j - 1] == 0 || B[i + 1][j] == 0 || B[i][j + 1] == 0)
					dst[i*cols + j] = 100;
			}
		}
	}*/
}