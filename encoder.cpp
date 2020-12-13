#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <bitset>
using namespace std;

class TreeNode
{
public:
	int ascii;
	int prob;
	TreeNode* leftChild, * rightChild;	// 왼쪽 자식 노드, 오른쪽 자식 노드
	int root_idx;	// 자신을 가리키는 root_array의 index
	char code;		// 0 또는 1의 codeword
	TreeNode* parent;	// 부모 노드

	// 생성자
	TreeNode(int _ascii, int _prob, TreeNode* _leftChild, TreeNode* _rightChild, int _root_idx, char _code, TreeNode* _parent)
	{
		ascii = _ascii;
		prob = _prob;
		leftChild = _leftChild;
		rightChild = _rightChild;
		root_idx = _root_idx;
		code = _code;
		parent = _parent;
	}
};

void huffmanTree_codeword(int*, char**, TreeNode**);
int* min_idx(int*, int);
void recursive_code(TreeNode*, TreeNode*, char, TreeNode**);
void get_codeword(TreeNode*, char*);
float get_cost(int, int, int, int);
int MAX(int* );
void make_hbs_file(const char*, char*);
void encode(char*, char*, char**, char***, int**, int, int);


int main()
{
	int prob_norm[128] = { 0, };		// normal huffman table의 빈도(확률)를 세는 배열
	int prob_copy_norm[128] = { 0, };	// prob_norm 사본 배열
	char** codeword_norm = new char* [128];	// 허프만 코딩으로 얻은 codeword
	for (int i = 0; i < 128; i++) {
		codeword_norm[i] = new char[30];		// 이차원 배열 동적 할당
		memset(codeword_norm[i], 0, sizeof(char) * 30);	// 0으로 초기화
	}
	char*** codeword = new char** [128];	// 허프만 트리를 이용해 얻은 codeword
	for (int i = 0; i < 128; i++) {
		codeword[i] = new char* [128];
		for (int j = 0; j < 128; j++) {
			codeword[i][j] = new char[30];
			memset(codeword[i][j], 0, sizeof(char) * 30);
		}
	}
	int** prob = new int* [128];	 // context adaptive huffman table의 빈도(확률)를 세는 배열
	for (int i = 0; i < 128; i++) {	 // row: preceding symbol, col: symbol
		prob[i] = new int[128];
		memset(prob[i], 0, sizeof(int) * 128);	// 0으로 초기화
	}
	int** prob_copy = new int* [128];		// prob 사본 배열
	for (int i = 0; i < 128; i++) {
		prob_copy[i] = new int[128];
		memset(prob_copy[i], 0, sizeof(int) * 128);	// 0으로 초기화
	}
	
	TreeNode* root_array_norm[128] = { 0, };	// 여러 임시 root를 저장하는 배열
	TreeNode* root_array[128][128] = { 0, };		
	char* encoding = new char[100000];			// 허프만 인코딩한 문자열을 담는 배열
	memset(encoding, 0, sizeof(char) * 100000);
	char* encoding_table_norm = new char[100000];		// normal허프만 테이블을 담는 배열
	memset(encoding_table_norm, 0, sizeof(char) * 100000);
	char* encoding_table = new char[100000];			// adpative 허프만 테이블을 담는 배열
	memset(encoding_table, 0, sizeof(char) * 100000);
	int EOD_ascii = 36;		// 임의로 정한 EOD의 아스키 값('$')
	char EOD[20] = { 0, };	// EOD의 codeword

	char ch = 0;
	int pre_ascii = 0, ascii = 0;
	const char* c_str = 0;
	string str;
	int codeword_bit = 0;

	char* file_data = new char[100000];	// 파일의 데이터를 저장할 배열, 메모리 할당 후 0으로 초기화
	memset(file_data, 0, sizeof(char) * 100000);


	ifstream fin_training("training_input.txt");	// "training_input.txt" 파일을 열어 읽어오기
	if (!fin_training.is_open())	// 파일이 열리지 않을 경우 프로그램 종료
	{
		printf("File can't be opened!\n");
		return -1;
	}

	fin_training.seekg(0, fin_training.end);
	int uncompress_byte = (int)fin_training.tellg();	// 원본 파일의 바이트 수 
	fin_training.seekg(0, fin_training.beg);

	int len;
	for (len = 0; !fin_training.eof(); len++) {
		fin_training.get(file_data[len]);	// fin에서 한 글자씩 읽어와  file_data에 저장
	}
	file_data[--len] = 0;	// EOF 앞 널 문자를 len에서 뺌
	fin_training.close();


	// For Normal Huffman table
	for (int i = 0; i < len; i++) {
		ascii = (int)file_data[i];	// 각 문자의 아스키 코드 값
		prob_norm[ascii]++;		// 아스키 코드 값에 해당하는 index에서의 값을 1 증가시켜 카운팅
	}
	prob_norm[EOD_ascii]++;		// EOD를 한번 넣어줌 (EOD: '$')

	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		prob_norm[i]++;
		prob_copy_norm[i] = prob_norm[i];	// prob_norm의 값을 복사한 prob_copy_norm
	}

	huffmanTree_codeword(prob_copy_norm, codeword_norm, root_array_norm);	// 허프만 트리를 생성하고 codeword 저장
	// 허프만테이블 출력
	//for (int i = 0; i < 128; i++) {
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;
	//	/*if (*codeword_norm[i] == 0)
	//		continue;*/
	//	printf("[%d]'%c'\t\t%d\t\t%s\n", i, i, prob_norm[i], codeword_norm[i]);	// Symbol, probability, codeword
	//}


	// EOD
	strcpy(EOD, codeword_norm[EOD_ascii]);	// EOD codeword 저장


	// For Context adaptive huffman table
	char pre_ch = file_data[0];		// preceding symbol
	pre_ascii = (int)pre_ch;	// ascii of preceding symbol
	for (int i = 1; i < len; i++) {
		ch = file_data[i];		// symbol
		ascii = (int)ch;			// ascii of symbol

		/*if (pre_ascii == 10 || pre_ascii == 32 || pre_ascii == 44 || pre_ascii == 46 || pre_ascii == 63
			|| pre_ascii == 65 || pre_ascii == 69 || pre_ascii == 79 || pre_ascii == 82 || pre_ascii == 84
			|| pre_ascii == 85 || pre_ascii == 86 || pre_ascii == 96 || pre_ascii == 97 || pre_ascii == 98
			|| pre_ascii == 99 || pre_ascii == 100 || pre_ascii == 101 || pre_ascii == 104
			|| pre_ascii == 105 || pre_ascii == 108 || pre_ascii == 109 || pre_ascii == 110
			|| pre_ascii == 111 || pre_ascii == 114 || pre_ascii == 115 || pre_ascii == 116
			|| pre_ascii == 117 || pre_ascii == 118 || pre_ascii == 119 || pre_ascii == 121) // 압축률(C) 최소
				prob[pre_ascii][ascii]++;
		*/
		if(pre_ascii == 32)	// Cost 최소
			prob[pre_ascii][ascii]++;

		pre_ascii = ascii;
	}

	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		for (int j = 0; j < 128; j++) {
			if ((j < 10) || (j > 10 && j < 32) || (j == 127))
				continue;

			prob[i][j] += 1;	// prob의 최소값을 1로 만듦
			prob_copy[i][j] = prob[i][j];		// prob의 값을 복사한 prob_copy
		}
	}
	// prob_copy 출력
	//for (int i = 0; i < 128; i++) {
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;
	//	for (int j = 0; j < 128; j++) {
	//		if ((j < 10) || (j > 10 && j < 32) || (j == 127))
	//			continue;
	//		if (prob_copy[i][j] == 1)
	//			continue;
	//		printf("[%d][%d]: %d\n", i, j, prob_copy[i][j]);		// prob의 값을 복사한 prob_copy
	//	}
	//}

	int max = 0;
	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		// 모든 빈도수가 0인 adaptive huffman code table은 건너뜀
		max = MAX(prob_copy[i]);	// 최대값 찾음
		if (max == 1)	// 최대값이 1이면 모든 prob이 1이므로 건너뜀
			continue;

		huffmanTree_codeword(prob_copy[i], codeword[i], root_array[i]);	// 허프만 트리를 생성하고 codeword 저장
	}
	// context adaptive huffman table 출력
	//for (int i = 0; i < 128; i++) {	// preceding symbol
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;

	//	max = MAX(prob_copy[i]);	// 최대값 찾음
	//	if (max == 1)	// 최대값이 1이면 모든 prob이 1이므로 건너뜀
	//		continue;

	//	for (int j = 0; j < 128; j++) {	// symbol
	//		if ((j < 10) || (j > 10 && j < 32) || (j == 127))
	//			continue;

	//		printf("'%c''%c'\t\t%d\t\t%s\n", i, j, prob[i][j], codeword[i][j]);	// symbol, probability, codeword
	//	}
	//	printf("\n");
	//}


	// adaptive table에서 bit 수가 큰 테이블을 삭제하기 위해 bit 수 카운팅
	int adaptive_bit[128] = { 0, };
	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < 128; j++) {
			codeword_bit = strlen(codeword[i][j]);
			adaptive_bit[i] += (codeword_bit * prob[i][j]);	// (codeword의 bit 수 * 빈도 수)를 더함
		}
	}
	//for (int i = 0; i < 128; i++) {
	//	if (adaptive_bit[i] == 0)
	//		continue;
	//	printf("'%d' : %d\n", i, adaptive_bit[i]);
	//}


	// training_input.txt를 encoding
	encode(file_data, encoding, codeword_norm, codeword, prob, EOD_ascii, len);


	// normal 허프만 테이블
	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		// ascii 코드 써넣기
		bitset<8> bs1(i);			// 10진수를 8bit binary로 변환
		str = bs1.to_string();
		c_str = str.c_str();	// string to const char*
		strcat(encoding_table_norm, c_str);

		// codeword의 bit 수 써넣기
		codeword_bit = strlen(codeword_norm[i]);
		bitset<8> bs2(codeword_bit);	// 10진수를 binary로 변환
		str = bs2.to_string();
		c_str = str.c_str();	// string to const char*
		strcat(encoding_table_norm, c_str);

		// codeword 써넣기
		strcat(encoding_table_norm, codeword_norm[i]);
	}

	strcat(encoding_table_norm, EOD);	// encoding_table_norm 끝에 EOD 삽입

	int norm_table_bit = strlen(encoding_table_norm);	// normal 허프만 테이블의 bit 수
	int stuffing_num = 8 - (norm_table_bit % 8);	// 끝 부분을 8bit로 만들기 위해 0으로 채울 비트의 수 
	for (int i = 0; i < stuffing_num; i++) {
		if (stuffing_num == 8)		// 끝 8bit가 원래 다 채워져있으면 탈출
			break;
		encoding_table_norm[norm_table_bit++] = '0';
	}
	encoding_table_norm[norm_table_bit] = '\0';



	// context adaptive 허프만 테이블
	// 순서 : preceding ascii 7bit, ascii 8bit, codeword bit수 8bit, codeword
	for (int i = 1; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;
		max = MAX(prob[i]);	// 최대값 찾음
		if (max == 1)	// 최대값이 1이면 모든 prob이 1이므로 건너뜀
			continue;

		for (int j = 1; j < 128; j++) {
			if ((j < 10) || (j> 10 && j < 32) || (j == 127))
				continue;
			// preceding symbol의 ascii 코드 써넣기
			bitset<7> bs0(i);			// 10진수를 7bit binary로 변환
			str = bs0.to_string();
			c_str = str.c_str();	// string to const char*
			strcat(encoding_table, c_str);

			// symbol의 ascii 코드 써넣기
			bitset<8> bs1(j);			// 10진수를 8bit binary로 변환
			str = bs1.to_string();
			c_str = str.c_str();	// string to const char*
			strcat(encoding_table, c_str);

			// codeword의 bit 수 써넣기
			codeword_bit = strlen(codeword[i][j]);	// codeword의 bit 수
			bitset<8> bs2(codeword_bit);	// 10진수를 binary로 변환
			str = bs2.to_string();
			c_str = str.c_str();	// string to const char*
			strcat(encoding_table, c_str);

			// codeword 써넣기
			strcat(encoding_table, codeword[i][j]);
		}
	}
	
	strcat(encoding_table, EOD);	// encoding_table 끝에 EOD 삽입

	int adaptive_table_bit = strlen(encoding_table);	// adaptive 허프만 테이블의 bit 수
	stuffing_num = 8 - (adaptive_table_bit % 8);	// 끝 부분을 8bit로 만들기 위해 0으로 채울 비트의 수 
	for (int i = 0; i < stuffing_num; i++) {
		if (stuffing_num == 8)		// 끝 8bit가 원래 다 채워져있으면 탈출
			break;
		encoding_table[adaptive_table_bit++] = '0';
	}
	encoding_table[adaptive_table_bit] = '\0';



	// Cost 계산
	int compress_byte = strlen(encoding) / 8;	// 인코딩한 파일의 바이트 수
	float cost = get_cost(uncompress_byte, compress_byte, norm_table_bit/8, adaptive_table_bit/8);


	// huffman_table.hbs 파일 생성
	make_hbs_file("huffman_table.hbs", encoding_table_norm);

	// context_adaptive_huffman_table.hbs 파일 생성
	make_hbs_file("context_adaptive_huffman_table.hbs", encoding_table);


	// training_input_code.hbs 파일 생성
	make_hbs_file("training_input_code.hbs", encoding);


	// test_input.txt 불러와서 file_data에 저장
	memset(file_data, 0, sizeof(char) * 100000);	// file_data 비움
	ifstream fin_test1("test_input.txt");	// "test_input.txt" 파일을 열어 읽어오기
	if (!fin_test1.is_open())	// 파일이 열리지 않을 경우 프로그램 종료
		return -1;
	for (len = 0; !fin_test1.eof(); len++) {
		fin_test1.get(file_data[len]);	// fin에서 한 글자씩 읽어와 file_data에 저장
	}
	file_data[--len] = 0;	// EOF 앞 널 문자를 len에서 뺌
	fin_test1.close();
	// file_data를 encoding
	memset(encoding, 0, sizeof(char) * 100000);		// encoding 비움
	encode(file_data, encoding, codeword_norm, codeword, prob, EOD_ascii, len);
	// test_input_code.hbs 파일 생성
	make_hbs_file("test_input_code.hbs", encoding);



	//메모리 해제
	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < 128; j++)
			delete[] codeword[i][j];
	}
	for (int i = 0; i < 128; i++) {
		delete[] codeword[i];
	}
	delete[] codeword;
}


// 허프만 트리를 생성하고 codeword를 저장하는 함수
void huffmanTree_codeword(int* prob_copy, char** codeword, TreeNode** root_array)
{
	int prob_root[128] = { 0, };		// root_array의 임시 root들이 가리키고 있는 TreeNode의 prob을 저장
	int* min_idx_prob = NULL;			// prob_copy 배열에서 최소값 두개의 index 
	int* min_idx_root = NULL;			// root_array 배열에서 최소값 두개의 index
	int* min_idx_prob_root = NULL;		// min_prob_root에서 최소값 두개의 index
	int min_prob_root[4] = { 0, };		// prob_copy 최소값 두개와 prob_root 최소값 두개를 차례로 저장
	int idx0, idx1, idx2, idx3, idx4, idx5;

	// Tree가 비어있는 초기 상태
	min_idx_prob = min_idx(prob_copy, 128);	// prob_copy에서 최소값 두개의 index 저장
	idx0 = min_idx_prob[0];
	idx1 = min_idx_prob[1];
	root_array[0] = new TreeNode(idx0, prob_copy[idx0], NULL, NULL, 0, '0', 0);	// prob: 최소값1, root0가 가리킴
	root_array[1] = new TreeNode(idx1, prob_copy[idx1], NULL, NULL, 1, '0', 0);	// prob: 최소값2, root1이 가리킴
	prob_copy[idx0] = prob_copy[idx1] = 0;		// TreeNode가 만들어졌으므로 해당 값 없앰

	root_array[0] = new TreeNode(0, root_array[0]->prob + root_array[1]->prob, root_array[0], root_array[1], 0, '0', 0);	// prob: 최소값1+최소값2, root0가 가리킴
	root_array[1] = 0;
	root_array[0]->leftChild->root_idx = -1;	// 자신을 가리키던 root가 다른걸 가리키므로 root_idx = -1로 설정
	root_array[0]->rightChild->root_idx = -1;
	root_array[0]->leftChild->parent = root_array[0]->rightChild->parent = root_array[0];	// 새로운 노드를 두 노드의 parent로 설정

	while (1) {
		// prob_copy에서 최소값 두개의 index 저장
		min_idx_prob = min_idx(prob_copy, 128);
		idx0 = min_idx_prob[0];
		idx1 = min_idx_prob[1];
		min_prob_root[0] = prob_copy[idx0];
		min_prob_root[1] = prob_copy[idx1];

		// prob_root에서 최소값 두개의 index 저장
		for (int i = 0; i < 128; i++) {	  // 임시 root들의 prob만을 저장
			prob_root[i] = 0;	// 0으로 초기화
			if (root_array[i] == 0)
				continue;
			prob_root[i] = root_array[i]->prob;
		}
		min_idx_root = min_idx(prob_root, 128);
		idx2 = min_idx_root[0];
		idx3 = min_idx_root[1];
		min_prob_root[2] = prob_root[idx2];
		min_prob_root[3] = prob_root[idx3];

		// min_prob_root에서 최소값 두개의 index 반환
		min_idx_prob_root = min_idx(min_prob_root, 4);
		idx4 = min_idx_prob_root[0];
		idx5 = min_idx_prob_root[1];
		if (min_prob_root[idx5] == 0) // prob_copy의 모든 값이 비어있고 최종 root 값 하나만 남은 경우 반복문 탈출
			break;

		int root_index[2] = { 0, };	// 덧셈할 두 노드를 가리키는 root_array의 index
		for (int i = 0; i < 2; i++) {
			int idx = (i == 0) ? idx4 : idx5;

			if (idx == 0 || idx == 1) {	// prob_copy의 값인 경우
				for (int j = 0; j < 128; j++) {
					if (root_array[j] == 0) // 비어있는 root의 index를 찾음
					{
						root_index[i] = j;		 // 새로 생성할 노드를 가리킬 root_array의 index

						int ascii = (idx == 0) ? idx0 : idx1;
						root_array[j] = new TreeNode(ascii, min_prob_root[idx], NULL, NULL, j, '0', 0);	// prob_copy의 값을 담은 TreeNode 생성
						idx = (idx == 0) ? idx0 : idx1;
						prob_copy[idx] = 0;			// TreeNode가 만들어졌으므로 해당 값 없앰
						break;
					}
				}
			}
			else 		// prob_root의 값인 경우
				root_index[i] = (idx == 2) ? idx2 : idx3;	// prob_root에서의 index = root_array에서의 index
		}

		// 두 노드를 더한 부모 노드 생성
		int root_index0 = root_index[0];
		int root_index1 = root_index[1];
		int root_idx = (root_index0 < root_index1) ? root_index0 : root_index1;	// 더 작은 root_index 값을 얻음, 상위 노드를 가리키기 위함
		root_array[root_idx] = new TreeNode(0, root_array[root_index0]->prob + root_array[root_index1]->prob, root_array[root_index0], root_array[root_index1], root_idx, '0', 0);	// 두 노드의 상위 노드 생성
		root_array[root_idx]->leftChild->root_idx = -1;	// 자식노드를 가리키던 root가 부모노드를 가리키므로 root_idx = -1로 설정
		root_array[root_idx]->rightChild->root_idx = -1;
		root_array[root_idx]->leftChild->parent = root_array[root_idx]->rightChild->parent = root_array[root_idx];	// 새로운 노드를 두 노드의 parent로 설정
		root_idx = (root_index0 > root_index1) ? root_index0 : root_index1;	// 더 큰 root_index 값을 얻음, 더 이상 가리키지 않으므로 NULL로 할당하기 위함
		root_array[root_idx] = 0;

	}

	recursive_code(0, root_array[0], '2', root_array);	// root 노드를 넘겨줌, root의 code는 사용하지 않으므로 2로 설정

	for (int i = 0; i < 128; i++)
	{
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		if (root_array[i] == NULL)
			continue;
		get_codeword(root_array[i], codeword[i]);	// 해당 아스키 값을 갖는 Symbol의 codeword를 구함
	}
}

int* min_idx(int* arr, int size)	// arr에서 0보다 큰 최소값의 index를 반환하는 함수
{
	int min_2_index[2] = { 0, };	// 최소값 2개의 index (반환 값)
	int min1, min2;
	int min_index1, min_index2;

	for (int i = 0; i < 2; i++) {
		min1 = arr[0];
		min_index1 = 0;	// 최소값의 index
		for (int j = 1; j < size; j++)
		{
			if (min1 == 0) {		// min이 0이면 무조건 arr[j]로 할당
				min1 = arr[j];
				min_index1 = j;
			}
			else {
				if (min1 > arr[j] && arr[j] != 0) {	// arr[j]가 min보다 작은 경우
					min1 = arr[j];
					min_index1 = j;
				}
			}
		}

		if (i == 0) {	// 첫번째
			min_2_index[0] = min_index1;	// 첫번째 최소값의 index 저장
			min2 = min1;
			min_index2 = min_index1;
			arr[min_index1] = 0;	// 첫번째 최소값을 0으로 설정하여 두번째 최소값 계산시 제외시킴
		}
		else {	// 두번째
			min_2_index[1] = min_index1;	// 두번째 최소값의 index 저장
			arr[min_index2] = min2;		// 첫번째 최소값을 0으로 설정해둔 것을 복구
		}
	}

	return min_2_index;
}

void recursive_code(TreeNode* parent, TreeNode* node, char code, TreeNode** root_array)	// 허프만 트리의 각 노드에 0 또는 1을 할당
{
	node->code = code;		// 0 또는 1의 code 값 설정 (root인 경우만 2)
	node->parent = parent;	// 부모 노드 저장

	if (node->leftChild == 0 && node->rightChild == 0) {	// leaf node
		int idx = node->ascii;
		root_array[idx] = node;

		return;

	}
	recursive_code(node, node->leftChild, '0', root_array);	// 왼쪽 자식노드는 code가 0
	recursive_code(node, node->rightChild, '1', root_array);	// 오른쪽 자식노드는 code가 1
}

void get_codeword(TreeNode* node, char* codeword)
{
	char reverse_codeword[30] = { 0, };	// leaf에서 root 방향으로 저장하므로 역순
	int	len;	// codeword의 길이
	for (len = 0; node->code != '2'; len++) { // root에 도달하면 종료, root의 code 저장 X
		reverse_codeword[len] = node->code;
		node = node->parent;	// 부모 노드로 이동
	}
	for (int i = 0; i < len; i++)
		codeword[i] = reverse_codeword[len - 1 - i];	// root에서 leaf 방향으로의 codeword
}

float get_cost(int uncompress_byte, int compress_byte, int norm_table_byte, int adaptive_table_byte)	// cost를 계산하는 함수
{
	float lagrange = 0.0001f;

	float C = (float)compress_byte / uncompress_byte;
	int N = norm_table_byte + adaptive_table_byte;
	float cost = C + (float)lagrange * N;

	printf("cost: %f\n", cost);

	return cost;
}

int MAX(int* prob)
{
	int max = prob[0];
	for (int j = 1; j < 128; j++) {
		if (prob[j] > max)
			max = prob[j];
	}
	return max;
}


// file_data를 encoding하여 encoding 배열에 저장하는 함수
void encode(char* file_data, char* encoding, char** codeword_norm, char*** codeword, int** prob, int EOD_ascii, int len)
{
	int pre_ascii = (int)file_data[0];
	int ascii = 0;
	strcat(encoding, codeword_norm[pre_ascii]);	// 맨 첫 글자는 normal table의 codeword를 사용
	
	int max = 0;
	for (int i = 1; i < len; i++) {
		ascii = (int)file_data[i];

		max = MAX(prob[pre_ascii]);
		if (max == 1)	// pre_ascii에 대한 adaptive table이 없는 경우 normal table 사용
			strcat(encoding, codeword_norm[ascii]);
		else  // adaptive table 사용
			strcat(encoding, codeword[pre_ascii][ascii]);	// 해당 문자의 codeword를 붙여쓰기

		pre_ascii = ascii;
	}
	max = MAX(prob[pre_ascii]);	// 맨 마지막 문자에 대한 adaptive table 있는지 확인
	if (max == 1)	// normal table
		strcat(encoding, codeword_norm[EOD_ascii]);	// normal table의 EOD 삽입
	else           // adaptive table
		strcat(encoding, codeword[pre_ascii][EOD_ascii]);	// adaptive table의 EOD 삽입


	int size = strlen(encoding);	// encoding 배열의 전체 데이터 수 
	int stuffing_num = 8 - (size % 8);	// 끝 부분을 8bit로 만들기 위해 0으로 채울 비트의 수 
	for (int i = 0; i < stuffing_num; i++) {
		if (stuffing_num == 8)		// 끝 8bit가 원래 다 채워져있으면 탈출
			break;
		encoding[size++] = '0';	// byte align
	}
	encoding[size] = '\0';
}

void make_hbs_file(const char* file_name, char* encoding) {	// 8bit씩 파일에 write하는 함수

	ofstream fout_code(file_name, ios::binary);
	char buffer[9] = { 0, };

	int cnt = 0, num = 0;
	while (encoding[num] != '\0') {
		int index = 0;
		for (num = 8 * cnt; num < 8 * cnt + 8; num++) {	// 8bit씩 읽음
			buffer[index] = encoding[num];	// 8bit 문자열 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가

		int decimal = strtol(buffer, NULL, 2);	// 문자열을 10진수로 변환
		char ch = (char)decimal;	// 10진수를 -127~128의 char형으로 변환

		fout_code << ch;	// -127~128의 char형을 파일에 write하면 16진수로 저장됨

		cnt++;
	}
	fout_code.close();
}
