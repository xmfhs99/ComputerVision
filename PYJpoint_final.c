#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "PYJiplib.h"
#include "PYJpoint.h"

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
				disper20 += pow((j - centerx), 2.0)/area;
				disper02 += pow((i - centery), 2.0)/area;
				disper11 += (j - centerx) * (i - centery)/area;
			}
		}
	}

	r1 = sqrt(2 * (disper20 + disper02 + sqrt(pow((disper02 - disper20), 2) + 4 * disper11*disper11))); //r1���� ����
	r2 = sqrt(2 * (disper20 + disper02 - sqrt(pow((disper02 - disper20), 2) + 4 * disper11*disper11))); //r2���� ����
	orient = atan2(2 * disper11, (disper20 - disper02)) * (-0.5) * (180 / PI); //orientation
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

void connected_labelling(unsigned char *src, unsigned char *dst, int number_of_pixels, int rows, int cols) {
	FILE* fp;
	int **B = malloc(sizeof(int *) * rows); //2�� ���� 2���� �迭
	int **labelled = malloc(sizeof(int *) * rows); //�� �ȼ��� ������, union-find�� Ȱ��
	int *parent, *rank; //union-find
	int *area_array; //���� ������(unassembled)
	long *circles_x_center, *circles_y_center, *circle; //�� ���� x,y ��ǥ, circle => ���� ������(assembled)
	int *head_node_number; //�� ������ root node�� ��ȣ(union-find)

	int label = 0;
	int flag; //max_node ���� ����
	int max_node; //labelled�� �ִ밪, union-find�� ���� ������ ���
	int count_circle; //���󿡼��� ���� ����

	//B, labelled �����Ҵ�
	for (int i = 0; i < rows; i++) B[i] = malloc(sizeof(char *) * cols); 
	for (int i = 0; i < rows; i++) labelled[i] = malloc(sizeof(char *) * cols);

	//2�� ���� 2���� �迭 B ����
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (src[i*cols + j] >= 100) { //threshhold = 100
				B[i][j] = 1;  dst[i*cols + j] = 255; //���� ���� B, ��¿��� 255
			}
			else {
				B[i][j] = 0;  dst[i*cols + j] = 0;
			}
			labelled[i][j] = 0; //�� �迭�� ���� �ʱ�ȭ
		}
	}

	flag = 0;
	max_node=0; //��� ��ȣ �ʱ�ȭ
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (B[i][j] != 0) {
				if (!flag) {
					labelled[i][j] = ++max_node; //���� 1���� ����(�ʱ�ȭ�� 0���� �߱� ����)
					flag = 1; //�̾����� �ȼ������� ���� max_node���� �����Ű�� ����(���� ����)
				}
				else labelled[i][j] = max_node; //flag�� ����
			}
			else flag = 0; //�ȼ��� �̾����ٰ� ���� ����� �ٽ� flag=0���� ����
		}
	}

	//using the file stream to see how your code works
	//�ַܼδ� ���� �� ���� Ȯ���� �ȵǼ� �޸����� �̿��ؼ� Ȯ��
	//�� �����̸� 1��, ����̸� 0���� ǥ��
	fp = fopen("seehow.txt", "w");
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if(labelled[i][j] != 0)
				fputc('1', fp);
			else fputc('0', fp);
		}
		fputc('\n', fp);
	}
	fclose(fp);

	//union-find �۾��� ���� �ʱ�ȭ, ũ��� max_node+1 ���� (0~max_node)
	parent = malloc(sizeof(int) * (max_node+1));
	rank = malloc(sizeof(int) * (max_node+1));
	for (int i = 0; i <= max_node; i++) { parent[i] = i; rank[i] = 0; }

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (B[i][j] != 0) {
				if (labelled[i + 1][j] != 0 && labelled[i][j] != labelled[i + 1][j]) { //4 - neighbors
					uni(parent, rank, labelled[i][j], labelled[i + 1][j]); //�� �ȼ��� �Ʒ� �ȼ�(labelled[i+1][j])�� �θ���� ���� 
				}
				if (labelled[i + 1][j + 1] != 0 && labelled[i][j] != labelled[i + 1][j + 1]) { //8 - neighbors
					uni(parent, rank, labelled[i][j], labelled[i + 1][j + 1]);//�ϵ��� �ȼ�(labelled[i+1][j+1])�� �Ʒ� �ȼ�(labelled[i+1][j])�� �θ���� ���� 
				}
			}
		}
	}
	area_array = malloc(sizeof(int) * (max_node+1)); //������ ���ϵ�, find �Լ����� �Ἥ �� �ȼ��� ������Ű�Ƿ�, size�� ��� ũ��� ����
	for (int i = 0; i <= max_node; i++) area_array[i] = 0; //�ʱ�ȭ

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if(labelled[i][j] != 0) area_array[find(parent, labelled[i][j])]++; //�� ����� root��常 1���� ����
		}
	}

	//�� ���� Ȯ��
	/*for (int i = 0; i <= max_node; i++) {
		printf("%d, %d\n", i, area_array[i]);
	}*/

	count_circle = 0; //���� ����
	head_node_number = malloc(sizeof(int) * (max_node+1)); //�� �θ���(�θ𿵿�)�� �� ��° ���ΰ�
	for (int i = 0; i < max_node; i++) head_node_number[i] = 0; //�ʱ�ȭ

	for (int i = 0; i < max_node; i++) {
		if (area_array[i] != 0) { //������ 0�� �ƴ� ��, �� ���� ���� ������ �ִ� �θ𿵿�(i)�� ��쿡��
			head_node_number[i] = count_circle++; //������� ���� ��ȣ (#)�� ����
		}
	}
	printf("cnt of circle : %d\n", count_circle); //���� ���� Ȯ��

	circle = malloc(sizeof(int) * count_circle); //�� ���� ����, area_array�� max_node�� ������� ������ ���̱� ������ index�� ������ ����
	count_circle = 0;
	for (int i = 0; i < max_node; i++) {
		if (area_array[i] != 0) circle[count_circle++] = area_array[i]; //�̸� �� ���� ��ȣ���� ������� ������ ��������
	}

	circles_x_center = malloc(sizeof(long) * count_circle); //�� ���� x��ǥ���� ��
	circles_y_center = malloc(sizeof(long) * count_circle); //�� ���� y��ǥ���� ��
	for (int i = 0; i < count_circle; i++) { //�ʱ�ȭ
		circles_x_center[i] = 0;
		circles_y_center[i] = 0;
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (labelled[i][j] != 0) { //�� ���� �ȼ��̸�, �� ����� �ƴϸ�
				circles_x_center[head_node_number[find(parent, labelled[i][j])]] += j; //���
				circles_y_center[head_node_number[find(parent, labelled[i][j])]] += i;
			}
		}
	}

	for (int i = 0; i < count_circle; i++) {
		circles_x_center[i] /= circle[i]; //�� ���� x,y ��ǥ���� ���� �� ���� ���������� ����
		circles_y_center[i] /= circle[i]; 

		printf("%d# ���� �߽���ǥ : (%d, %d)\n", i+1, circles_x_center[i], circles_y_center[i]);
		printf("%d# ���� ���� : %d\n", i+1, circle[i]);
		printf("\n");
	}

	//dst�� ���� �߽ɸ��� ���ڰ� ǥ���ϱ�
	for (int i = 0; i < count_circle; i++) {
		for (int j = 0; j < 15; j++) {
			dst[(circles_y_center[i] + j - 7) * cols + circles_x_center[i]] = 110;
			dst[circles_y_center[i] * cols + circles_x_center[i] + j - 7] = 110;
		}
	}

	//���� �޸� ����
	for (int y = 0; y < rows; y++) {
		free(*(B + y));
		free(*(labelled + y));
	}
	free(B);
	free(labelled);
	free(area_array);
	free(circles_x_center);
	free(circles_y_center);
	free(circle);
}

//union-find�� union �Լ� y�� �θ���� x�� ����
//rank�� �� ������ ����(level), �ð����⵵ ȿ���� ���븦 ����
void uni(int *parent, int *rank, int x, int y) {
	x = find(parent, x);
	y = find(parent, y);
	if (x == y) return;
	if (rank[x] < rank[y]) parent[x] = y;
	else parent[y] = x;
	if (rank[x] == rank[y]) rank[x]++;
}
//union-find�� find �Լ�, ��͸� �̿��Ͽ� x�� �θ��带 ã��
int find(int *parent, int x) {
	if (parent[x] == x) return x;
	return parent[x] = find(parent, parent[x]);
}