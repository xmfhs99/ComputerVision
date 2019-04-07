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
				disper20 += pow((j - centerx), 2.0)/area;
				disper02 += pow((i - centery), 2.0)/area;
				disper11 += (j - centerx) * (i - centery)/area;
			}
		}
	}

	r1 = sqrt(2 * (disper20 + disper02 + sqrt(pow((disper02 - disper20), 2) + 4 * disper11*disper11))); //r1장축 공식
	r2 = sqrt(2 * (disper20 + disper02 - sqrt(pow((disper02 - disper20), 2) + 4 * disper11*disper11))); //r2단축 공식
	orient = atan2(2 * disper11, (disper20 - disper02)) * (-0.5) * (180 / PI); //orientation
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

void connected_labelling(unsigned char *src, unsigned char *dst, int number_of_pixels, int rows, int cols) {
	FILE* fp;
	int **B = malloc(sizeof(int *) * rows); //2진 영상 2차원 배열
	int **labelled = malloc(sizeof(int *) * rows); //각 픽셀의 영역값, union-find에 활용
	int *parent, *rank; //union-find
	int *area_array; //원의 면적값(unassembled)
	long *circles_x_center, *circles_y_center, *circle; //각 원의 x,y 좌표, circle => 원의 면적값(assembled)
	int *head_node_number; //각 노드들의 root node의 번호(union-find)

	int label = 0;
	int flag; //max_node 증가 제어
	int max_node; //labelled의 최대값, union-find의 가장 마지막 노드
	int count_circle; //영상에서의 원의 갯수

	//B, labelled 동적할당
	for (int i = 0; i < rows; i++) B[i] = malloc(sizeof(char *) * cols); 
	for (int i = 0; i < rows; i++) labelled[i] = malloc(sizeof(char *) * cols);

	//2진 영상 2차원 배열 B 설정
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (src[i*cols + j] >= 100) { //threshhold = 100
				B[i][j] = 1;  dst[i*cols + j] = 255; //이진 영상 B, 출력영상 255
			}
			else {
				B[i][j] = 0;  dst[i*cols + j] = 0;
			}
			labelled[i][j] = 0; //라벨 배열도 같이 초기화
		}
	}

	flag = 0;
	max_node=0; //노드 번호 초기화
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (B[i][j] != 0) {
				if (!flag) {
					labelled[i][j] = ++max_node; //영역 1부터 시작(초기화를 0으로 했기 때문)
					flag = 1; //이어지는 픽셀에서는 같은 max_node값을 적용시키기 위함(증가 억제)
				}
				else labelled[i][j] = max_node; //flag의 목적
			}
			else flag = 0; //픽셀이 이어지다가 원을 벗어나면 다시 flag=0으로 설정
		}
	}

	//using the file stream to see how your code works
	//콘솔로는 값이 잘 들어갔나 확인이 안되서 메모장을 이용해서 확인
	//원 영역이면 1로, 배경이면 0으로 표시
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

	//union-find 작업을 위한 초기화, 크기는 max_node+1 까지 (0~max_node)
	parent = malloc(sizeof(int) * (max_node+1));
	rank = malloc(sizeof(int) * (max_node+1));
	for (int i = 0; i <= max_node; i++) { parent[i] = i; rank[i] = 0; }

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (B[i][j] != 0) {
				if (labelled[i + 1][j] != 0 && labelled[i][j] != labelled[i + 1][j]) { //4 - neighbors
					uni(parent, rank, labelled[i][j], labelled[i + 1][j]); //윗 픽셀을 아랫 픽셀(labelled[i+1][j])의 부모노드로 설정 
				}
				if (labelled[i + 1][j + 1] != 0 && labelled[i][j] != labelled[i + 1][j + 1]) { //8 - neighbors
					uni(parent, rank, labelled[i][j], labelled[i + 1][j + 1]);//북동쪽 픽셀(labelled[i+1][j+1])을 아랫 픽셀(labelled[i+1][j])의 부모노드로 설정 
				}
			}
		}
	}
	area_array = malloc(sizeof(int) * (max_node+1)); //면적을 구하되, find 함수만을 써서 각 픽셀을 증가시키므로, size는 노드 크기와 동일
	for (int i = 0; i <= max_node; i++) area_array[i] = 0; //초기화

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if(labelled[i][j] != 0) area_array[find(parent, labelled[i][j])]++; //각 노드의 root노드만 1개씩 증가
		}
	}

	//잘 들어갔나 확인
	/*for (int i = 0; i <= max_node; i++) {
		printf("%d, %d\n", i, area_array[i]);
	}*/

	count_circle = 0; //원의 갯수
	head_node_number = malloc(sizeof(int) * (max_node+1)); //각 부모노드(부모영역)은 몇 번째 원인가
	for (int i = 0; i < max_node; i++) head_node_number[i] = 0; //초기화

	for (int i = 0; i < max_node; i++) {
		if (area_array[i] != 0) { //면적이 0이 아닐 때, 즉 면적 값을 가지고 있는 부모영역(i)일 경우에만
			head_node_number[i] = count_circle++; //순서대로 원의 번호 (#)을 배정
		}
	}
	printf("cnt of circle : %d\n", count_circle); //원의 갯수 확이

	circle = malloc(sizeof(int) * count_circle); //각 원의 면적, area_array는 max_node를 기반으로 설정된 값이기 때문에 index의 편차가 심함
	count_circle = 0;
	for (int i = 0; i < max_node; i++) {
		if (area_array[i] != 0) circle[count_circle++] = area_array[i]; //이를 각 원의 번호마다 순서대로 면적을 지정해줌
	}

	circles_x_center = malloc(sizeof(long) * count_circle); //각 원의 x좌표값의 합
	circles_y_center = malloc(sizeof(long) * count_circle); //각 원의 y좌표값의 합
	for (int i = 0; i < count_circle; i++) { //초기화
		circles_x_center[i] = 0;
		circles_y_center[i] = 0;
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (labelled[i][j] != 0) { //원 내의 픽셀이면, 즉 배경이 아니면
				circles_x_center[head_node_number[find(parent, labelled[i][j])]] += j; //계산
				circles_y_center[head_node_number[find(parent, labelled[i][j])]] += i;
			}
		}
	}

	for (int i = 0; i < count_circle; i++) {
		circles_x_center[i] /= circle[i]; //각 원의 x,y 좌표값의 합을 각 원의 면적값으로 나눔
		circles_y_center[i] /= circle[i]; 

		printf("%d# 원의 중심좌표 : (%d, %d)\n", i+1, circles_x_center[i], circles_y_center[i]);
		printf("%d# 원의 넓이 : %d\n", i+1, circle[i]);
		printf("\n");
	}

	//dst에 원의 중심마다 십자가 표시하기
	for (int i = 0; i < count_circle; i++) {
		for (int j = 0; j < 15; j++) {
			dst[(circles_y_center[i] + j - 7) * cols + circles_x_center[i]] = 110;
			dst[circles_y_center[i] * cols + circles_x_center[i] + j - 7] = 110;
		}
	}

	//동적 메모리 해제
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

//union-find의 union 함수 y의 부모노드로 x를 설정
//rank는 각 집합의 깊이(level), 시간복잡도 효율성 증대를 위함
void uni(int *parent, int *rank, int x, int y) {
	x = find(parent, x);
	y = find(parent, y);
	if (x == y) return;
	if (rank[x] < rank[y]) parent[x] = y;
	else parent[y] = x;
	if (rank[x] == rank[y]) rank[x]++;
}
//union-find의 find 함수, 재귀를 이용하여 x의 부모노드를 찾음
int find(int *parent, int x) {
	if (parent[x] == x) return x;
	return parent[x] = find(parent, parent[x]);
}