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
	TreeNode* leftChild, * rightChild;	// ���� �ڽ� ���, ������ �ڽ� ���
	int root_idx;	// �ڽ��� ����Ű�� root_array�� index
	char code;		// 0 �Ǵ� 1�� codeword
	TreeNode* parent;	// �θ� ���

	// ������
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
	int prob_norm[128] = { 0, };		// normal huffman table�� ��(Ȯ��)�� ���� �迭
	int prob_copy_norm[128] = { 0, };	// prob_norm �纻 �迭
	char** codeword_norm = new char* [128];	// ������ �ڵ����� ���� codeword
	for (int i = 0; i < 128; i++) {
		codeword_norm[i] = new char[30];		// ������ �迭 ���� �Ҵ�
		memset(codeword_norm[i], 0, sizeof(char) * 30);	// 0���� �ʱ�ȭ
	}
	char*** codeword = new char** [128];	// ������ Ʈ���� �̿��� ���� codeword
	for (int i = 0; i < 128; i++) {
		codeword[i] = new char* [128];
		for (int j = 0; j < 128; j++) {
			codeword[i][j] = new char[30];
			memset(codeword[i][j], 0, sizeof(char) * 30);
		}
	}
	int** prob = new int* [128];	 // context adaptive huffman table�� ��(Ȯ��)�� ���� �迭
	for (int i = 0; i < 128; i++) {	 // row: preceding symbol, col: symbol
		prob[i] = new int[128];
		memset(prob[i], 0, sizeof(int) * 128);	// 0���� �ʱ�ȭ
	}
	int** prob_copy = new int* [128];		// prob �纻 �迭
	for (int i = 0; i < 128; i++) {
		prob_copy[i] = new int[128];
		memset(prob_copy[i], 0, sizeof(int) * 128);	// 0���� �ʱ�ȭ
	}
	
	TreeNode* root_array_norm[128] = { 0, };	// ���� �ӽ� root�� �����ϴ� �迭
	TreeNode* root_array[128][128] = { 0, };		
	char* encoding = new char[100000];			// ������ ���ڵ��� ���ڿ��� ��� �迭
	memset(encoding, 0, sizeof(char) * 100000);
	char* encoding_table_norm = new char[100000];		// normal������ ���̺��� ��� �迭
	memset(encoding_table_norm, 0, sizeof(char) * 100000);
	char* encoding_table = new char[100000];			// adpative ������ ���̺��� ��� �迭
	memset(encoding_table, 0, sizeof(char) * 100000);
	int EOD_ascii = 36;		// ���Ƿ� ���� EOD�� �ƽ�Ű ��('$')
	char EOD[20] = { 0, };	// EOD�� codeword

	char ch = 0;
	int pre_ascii = 0, ascii = 0;
	const char* c_str = 0;
	string str;
	int codeword_bit = 0;

	char* file_data = new char[100000];	// ������ �����͸� ������ �迭, �޸� �Ҵ� �� 0���� �ʱ�ȭ
	memset(file_data, 0, sizeof(char) * 100000);


	ifstream fin_training("training_input.txt");	// "training_input.txt" ������ ���� �о����
	if (!fin_training.is_open())	// ������ ������ ���� ��� ���α׷� ����
	{
		printf("File can't be opened!\n");
		return -1;
	}

	fin_training.seekg(0, fin_training.end);
	int uncompress_byte = (int)fin_training.tellg();	// ���� ������ ����Ʈ �� 
	fin_training.seekg(0, fin_training.beg);

	int len;
	for (len = 0; !fin_training.eof(); len++) {
		fin_training.get(file_data[len]);	// fin���� �� ���ھ� �о��  file_data�� ����
	}
	file_data[--len] = 0;	// EOF �� �� ���ڸ� len���� ��
	fin_training.close();


	// For Normal Huffman table
	for (int i = 0; i < len; i++) {
		ascii = (int)file_data[i];	// �� ������ �ƽ�Ű �ڵ� ��
		prob_norm[ascii]++;		// �ƽ�Ű �ڵ� ���� �ش��ϴ� index������ ���� 1 �������� ī����
	}
	prob_norm[EOD_ascii]++;		// EOD�� �ѹ� �־��� (EOD: '$')

	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		prob_norm[i]++;
		prob_copy_norm[i] = prob_norm[i];	// prob_norm�� ���� ������ prob_copy_norm
	}

	huffmanTree_codeword(prob_copy_norm, codeword_norm, root_array_norm);	// ������ Ʈ���� �����ϰ� codeword ����
	// ���������̺� ���
	//for (int i = 0; i < 128; i++) {
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;
	//	/*if (*codeword_norm[i] == 0)
	//		continue;*/
	//	printf("[%d]'%c'\t\t%d\t\t%s\n", i, i, prob_norm[i], codeword_norm[i]);	// Symbol, probability, codeword
	//}


	// EOD
	strcpy(EOD, codeword_norm[EOD_ascii]);	// EOD codeword ����


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
			|| pre_ascii == 117 || pre_ascii == 118 || pre_ascii == 119 || pre_ascii == 121) // �����(C) �ּ�
				prob[pre_ascii][ascii]++;
		*/
		if(pre_ascii == 32)	// Cost �ּ�
			prob[pre_ascii][ascii]++;

		pre_ascii = ascii;
	}

	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		for (int j = 0; j < 128; j++) {
			if ((j < 10) || (j > 10 && j < 32) || (j == 127))
				continue;

			prob[i][j] += 1;	// prob�� �ּҰ��� 1�� ����
			prob_copy[i][j] = prob[i][j];		// prob�� ���� ������ prob_copy
		}
	}
	// prob_copy ���
	//for (int i = 0; i < 128; i++) {
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;
	//	for (int j = 0; j < 128; j++) {
	//		if ((j < 10) || (j > 10 && j < 32) || (j == 127))
	//			continue;
	//		if (prob_copy[i][j] == 1)
	//			continue;
	//		printf("[%d][%d]: %d\n", i, j, prob_copy[i][j]);		// prob�� ���� ������ prob_copy
	//	}
	//}

	int max = 0;
	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		// ��� �󵵼��� 0�� adaptive huffman code table�� �ǳʶ�
		max = MAX(prob_copy[i]);	// �ִ밪 ã��
		if (max == 1)	// �ִ밪�� 1�̸� ��� prob�� 1�̹Ƿ� �ǳʶ�
			continue;

		huffmanTree_codeword(prob_copy[i], codeword[i], root_array[i]);	// ������ Ʈ���� �����ϰ� codeword ����
	}
	// context adaptive huffman table ���
	//for (int i = 0; i < 128; i++) {	// preceding symbol
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;

	//	max = MAX(prob_copy[i]);	// �ִ밪 ã��
	//	if (max == 1)	// �ִ밪�� 1�̸� ��� prob�� 1�̹Ƿ� �ǳʶ�
	//		continue;

	//	for (int j = 0; j < 128; j++) {	// symbol
	//		if ((j < 10) || (j > 10 && j < 32) || (j == 127))
	//			continue;

	//		printf("'%c''%c'\t\t%d\t\t%s\n", i, j, prob[i][j], codeword[i][j]);	// symbol, probability, codeword
	//	}
	//	printf("\n");
	//}


	// adaptive table���� bit ���� ū ���̺��� �����ϱ� ���� bit �� ī����
	int adaptive_bit[128] = { 0, };
	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < 128; j++) {
			codeword_bit = strlen(codeword[i][j]);
			adaptive_bit[i] += (codeword_bit * prob[i][j]);	// (codeword�� bit �� * �� ��)�� ����
		}
	}
	//for (int i = 0; i < 128; i++) {
	//	if (adaptive_bit[i] == 0)
	//		continue;
	//	printf("'%d' : %d\n", i, adaptive_bit[i]);
	//}


	// training_input.txt�� encoding
	encode(file_data, encoding, codeword_norm, codeword, prob, EOD_ascii, len);


	// normal ������ ���̺�
	for (int i = 0; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		// ascii �ڵ� ��ֱ�
		bitset<8> bs1(i);			// 10������ 8bit binary�� ��ȯ
		str = bs1.to_string();
		c_str = str.c_str();	// string to const char*
		strcat(encoding_table_norm, c_str);

		// codeword�� bit �� ��ֱ�
		codeword_bit = strlen(codeword_norm[i]);
		bitset<8> bs2(codeword_bit);	// 10������ binary�� ��ȯ
		str = bs2.to_string();
		c_str = str.c_str();	// string to const char*
		strcat(encoding_table_norm, c_str);

		// codeword ��ֱ�
		strcat(encoding_table_norm, codeword_norm[i]);
	}

	strcat(encoding_table_norm, EOD);	// encoding_table_norm ���� EOD ����

	int norm_table_bit = strlen(encoding_table_norm);	// normal ������ ���̺��� bit ��
	int stuffing_num = 8 - (norm_table_bit % 8);	// �� �κ��� 8bit�� ����� ���� 0���� ä�� ��Ʈ�� �� 
	for (int i = 0; i < stuffing_num; i++) {
		if (stuffing_num == 8)		// �� 8bit�� ���� �� ä���������� Ż��
			break;
		encoding_table_norm[norm_table_bit++] = '0';
	}
	encoding_table_norm[norm_table_bit] = '\0';



	// context adaptive ������ ���̺�
	// ���� : preceding ascii 7bit, ascii 8bit, codeword bit�� 8bit, codeword
	for (int i = 1; i < 128; i++) {
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;
		max = MAX(prob[i]);	// �ִ밪 ã��
		if (max == 1)	// �ִ밪�� 1�̸� ��� prob�� 1�̹Ƿ� �ǳʶ�
			continue;

		for (int j = 1; j < 128; j++) {
			if ((j < 10) || (j> 10 && j < 32) || (j == 127))
				continue;
			// preceding symbol�� ascii �ڵ� ��ֱ�
			bitset<7> bs0(i);			// 10������ 7bit binary�� ��ȯ
			str = bs0.to_string();
			c_str = str.c_str();	// string to const char*
			strcat(encoding_table, c_str);

			// symbol�� ascii �ڵ� ��ֱ�
			bitset<8> bs1(j);			// 10������ 8bit binary�� ��ȯ
			str = bs1.to_string();
			c_str = str.c_str();	// string to const char*
			strcat(encoding_table, c_str);

			// codeword�� bit �� ��ֱ�
			codeword_bit = strlen(codeword[i][j]);	// codeword�� bit ��
			bitset<8> bs2(codeword_bit);	// 10������ binary�� ��ȯ
			str = bs2.to_string();
			c_str = str.c_str();	// string to const char*
			strcat(encoding_table, c_str);

			// codeword ��ֱ�
			strcat(encoding_table, codeword[i][j]);
		}
	}
	
	strcat(encoding_table, EOD);	// encoding_table ���� EOD ����

	int adaptive_table_bit = strlen(encoding_table);	// adaptive ������ ���̺��� bit ��
	stuffing_num = 8 - (adaptive_table_bit % 8);	// �� �κ��� 8bit�� ����� ���� 0���� ä�� ��Ʈ�� �� 
	for (int i = 0; i < stuffing_num; i++) {
		if (stuffing_num == 8)		// �� 8bit�� ���� �� ä���������� Ż��
			break;
		encoding_table[adaptive_table_bit++] = '0';
	}
	encoding_table[adaptive_table_bit] = '\0';



	// Cost ���
	int compress_byte = strlen(encoding) / 8;	// ���ڵ��� ������ ����Ʈ ��
	float cost = get_cost(uncompress_byte, compress_byte, norm_table_bit/8, adaptive_table_bit/8);


	// huffman_table.hbs ���� ����
	make_hbs_file("huffman_table.hbs", encoding_table_norm);

	// context_adaptive_huffman_table.hbs ���� ����
	make_hbs_file("context_adaptive_huffman_table.hbs", encoding_table);


	// training_input_code.hbs ���� ����
	make_hbs_file("training_input_code.hbs", encoding);


	// test_input.txt �ҷ��ͼ� file_data�� ����
	memset(file_data, 0, sizeof(char) * 100000);	// file_data ���
	ifstream fin_test1("test_input.txt");	// "test_input.txt" ������ ���� �о����
	if (!fin_test1.is_open())	// ������ ������ ���� ��� ���α׷� ����
		return -1;
	for (len = 0; !fin_test1.eof(); len++) {
		fin_test1.get(file_data[len]);	// fin���� �� ���ھ� �о�� file_data�� ����
	}
	file_data[--len] = 0;	// EOF �� �� ���ڸ� len���� ��
	fin_test1.close();
	// file_data�� encoding
	memset(encoding, 0, sizeof(char) * 100000);		// encoding ���
	encode(file_data, encoding, codeword_norm, codeword, prob, EOD_ascii, len);
	// test_input_code.hbs ���� ����
	make_hbs_file("test_input_code.hbs", encoding);



	//�޸� ����
	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < 128; j++)
			delete[] codeword[i][j];
	}
	for (int i = 0; i < 128; i++) {
		delete[] codeword[i];
	}
	delete[] codeword;
}


// ������ Ʈ���� �����ϰ� codeword�� �����ϴ� �Լ�
void huffmanTree_codeword(int* prob_copy, char** codeword, TreeNode** root_array)
{
	int prob_root[128] = { 0, };		// root_array�� �ӽ� root���� ����Ű�� �ִ� TreeNode�� prob�� ����
	int* min_idx_prob = NULL;			// prob_copy �迭���� �ּҰ� �ΰ��� index 
	int* min_idx_root = NULL;			// root_array �迭���� �ּҰ� �ΰ��� index
	int* min_idx_prob_root = NULL;		// min_prob_root���� �ּҰ� �ΰ��� index
	int min_prob_root[4] = { 0, };		// prob_copy �ּҰ� �ΰ��� prob_root �ּҰ� �ΰ��� ���ʷ� ����
	int idx0, idx1, idx2, idx3, idx4, idx5;

	// Tree�� ����ִ� �ʱ� ����
	min_idx_prob = min_idx(prob_copy, 128);	// prob_copy���� �ּҰ� �ΰ��� index ����
	idx0 = min_idx_prob[0];
	idx1 = min_idx_prob[1];
	root_array[0] = new TreeNode(idx0, prob_copy[idx0], NULL, NULL, 0, '0', 0);	// prob: �ּҰ�1, root0�� ����Ŵ
	root_array[1] = new TreeNode(idx1, prob_copy[idx1], NULL, NULL, 1, '0', 0);	// prob: �ּҰ�2, root1�� ����Ŵ
	prob_copy[idx0] = prob_copy[idx1] = 0;		// TreeNode�� ����������Ƿ� �ش� �� ����

	root_array[0] = new TreeNode(0, root_array[0]->prob + root_array[1]->prob, root_array[0], root_array[1], 0, '0', 0);	// prob: �ּҰ�1+�ּҰ�2, root0�� ����Ŵ
	root_array[1] = 0;
	root_array[0]->leftChild->root_idx = -1;	// �ڽ��� ����Ű�� root�� �ٸ��� ����Ű�Ƿ� root_idx = -1�� ����
	root_array[0]->rightChild->root_idx = -1;
	root_array[0]->leftChild->parent = root_array[0]->rightChild->parent = root_array[0];	// ���ο� ��带 �� ����� parent�� ����

	while (1) {
		// prob_copy���� �ּҰ� �ΰ��� index ����
		min_idx_prob = min_idx(prob_copy, 128);
		idx0 = min_idx_prob[0];
		idx1 = min_idx_prob[1];
		min_prob_root[0] = prob_copy[idx0];
		min_prob_root[1] = prob_copy[idx1];

		// prob_root���� �ּҰ� �ΰ��� index ����
		for (int i = 0; i < 128; i++) {	  // �ӽ� root���� prob���� ����
			prob_root[i] = 0;	// 0���� �ʱ�ȭ
			if (root_array[i] == 0)
				continue;
			prob_root[i] = root_array[i]->prob;
		}
		min_idx_root = min_idx(prob_root, 128);
		idx2 = min_idx_root[0];
		idx3 = min_idx_root[1];
		min_prob_root[2] = prob_root[idx2];
		min_prob_root[3] = prob_root[idx3];

		// min_prob_root���� �ּҰ� �ΰ��� index ��ȯ
		min_idx_prob_root = min_idx(min_prob_root, 4);
		idx4 = min_idx_prob_root[0];
		idx5 = min_idx_prob_root[1];
		if (min_prob_root[idx5] == 0) // prob_copy�� ��� ���� ����ְ� ���� root �� �ϳ��� ���� ��� �ݺ��� Ż��
			break;

		int root_index[2] = { 0, };	// ������ �� ��带 ����Ű�� root_array�� index
		for (int i = 0; i < 2; i++) {
			int idx = (i == 0) ? idx4 : idx5;

			if (idx == 0 || idx == 1) {	// prob_copy�� ���� ���
				for (int j = 0; j < 128; j++) {
					if (root_array[j] == 0) // ����ִ� root�� index�� ã��
					{
						root_index[i] = j;		 // ���� ������ ��带 ����ų root_array�� index

						int ascii = (idx == 0) ? idx0 : idx1;
						root_array[j] = new TreeNode(ascii, min_prob_root[idx], NULL, NULL, j, '0', 0);	// prob_copy�� ���� ���� TreeNode ����
						idx = (idx == 0) ? idx0 : idx1;
						prob_copy[idx] = 0;			// TreeNode�� ����������Ƿ� �ش� �� ����
						break;
					}
				}
			}
			else 		// prob_root�� ���� ���
				root_index[i] = (idx == 2) ? idx2 : idx3;	// prob_root������ index = root_array������ index
		}

		// �� ��带 ���� �θ� ��� ����
		int root_index0 = root_index[0];
		int root_index1 = root_index[1];
		int root_idx = (root_index0 < root_index1) ? root_index0 : root_index1;	// �� ���� root_index ���� ����, ���� ��带 ����Ű�� ����
		root_array[root_idx] = new TreeNode(0, root_array[root_index0]->prob + root_array[root_index1]->prob, root_array[root_index0], root_array[root_index1], root_idx, '0', 0);	// �� ����� ���� ��� ����
		root_array[root_idx]->leftChild->root_idx = -1;	// �ڽĳ�带 ����Ű�� root�� �θ��带 ����Ű�Ƿ� root_idx = -1�� ����
		root_array[root_idx]->rightChild->root_idx = -1;
		root_array[root_idx]->leftChild->parent = root_array[root_idx]->rightChild->parent = root_array[root_idx];	// ���ο� ��带 �� ����� parent�� ����
		root_idx = (root_index0 > root_index1) ? root_index0 : root_index1;	// �� ū root_index ���� ����, �� �̻� ����Ű�� �����Ƿ� NULL�� �Ҵ��ϱ� ����
		root_array[root_idx] = 0;

	}

	recursive_code(0, root_array[0], '2', root_array);	// root ��带 �Ѱ���, root�� code�� ������� �����Ƿ� 2�� ����

	for (int i = 0; i < 128; i++)
	{
		if ((i < 10) || (i > 10 && i < 32) || (i == 127))
			continue;

		if (root_array[i] == NULL)
			continue;
		get_codeword(root_array[i], codeword[i]);	// �ش� �ƽ�Ű ���� ���� Symbol�� codeword�� ����
	}
}

int* min_idx(int* arr, int size)	// arr���� 0���� ū �ּҰ��� index�� ��ȯ�ϴ� �Լ�
{
	int min_2_index[2] = { 0, };	// �ּҰ� 2���� index (��ȯ ��)
	int min1, min2;
	int min_index1, min_index2;

	for (int i = 0; i < 2; i++) {
		min1 = arr[0];
		min_index1 = 0;	// �ּҰ��� index
		for (int j = 1; j < size; j++)
		{
			if (min1 == 0) {		// min�� 0�̸� ������ arr[j]�� �Ҵ�
				min1 = arr[j];
				min_index1 = j;
			}
			else {
				if (min1 > arr[j] && arr[j] != 0) {	// arr[j]�� min���� ���� ���
					min1 = arr[j];
					min_index1 = j;
				}
			}
		}

		if (i == 0) {	// ù��°
			min_2_index[0] = min_index1;	// ù��° �ּҰ��� index ����
			min2 = min1;
			min_index2 = min_index1;
			arr[min_index1] = 0;	// ù��° �ּҰ��� 0���� �����Ͽ� �ι�° �ּҰ� ���� ���ܽ�Ŵ
		}
		else {	// �ι�°
			min_2_index[1] = min_index1;	// �ι�° �ּҰ��� index ����
			arr[min_index2] = min2;		// ù��° �ּҰ��� 0���� �����ص� ���� ����
		}
	}

	return min_2_index;
}

void recursive_code(TreeNode* parent, TreeNode* node, char code, TreeNode** root_array)	// ������ Ʈ���� �� ��忡 0 �Ǵ� 1�� �Ҵ�
{
	node->code = code;		// 0 �Ǵ� 1�� code �� ���� (root�� ��츸 2)
	node->parent = parent;	// �θ� ��� ����

	if (node->leftChild == 0 && node->rightChild == 0) {	// leaf node
		int idx = node->ascii;
		root_array[idx] = node;

		return;

	}
	recursive_code(node, node->leftChild, '0', root_array);	// ���� �ڽĳ��� code�� 0
	recursive_code(node, node->rightChild, '1', root_array);	// ������ �ڽĳ��� code�� 1
}

void get_codeword(TreeNode* node, char* codeword)
{
	char reverse_codeword[30] = { 0, };	// leaf���� root �������� �����ϹǷ� ����
	int	len;	// codeword�� ����
	for (len = 0; node->code != '2'; len++) { // root�� �����ϸ� ����, root�� code ���� X
		reverse_codeword[len] = node->code;
		node = node->parent;	// �θ� ���� �̵�
	}
	for (int i = 0; i < len; i++)
		codeword[i] = reverse_codeword[len - 1 - i];	// root���� leaf ���������� codeword
}

float get_cost(int uncompress_byte, int compress_byte, int norm_table_byte, int adaptive_table_byte)	// cost�� ����ϴ� �Լ�
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


// file_data�� encoding�Ͽ� encoding �迭�� �����ϴ� �Լ�
void encode(char* file_data, char* encoding, char** codeword_norm, char*** codeword, int** prob, int EOD_ascii, int len)
{
	int pre_ascii = (int)file_data[0];
	int ascii = 0;
	strcat(encoding, codeword_norm[pre_ascii]);	// �� ù ���ڴ� normal table�� codeword�� ���
	
	int max = 0;
	for (int i = 1; i < len; i++) {
		ascii = (int)file_data[i];

		max = MAX(prob[pre_ascii]);
		if (max == 1)	// pre_ascii�� ���� adaptive table�� ���� ��� normal table ���
			strcat(encoding, codeword_norm[ascii]);
		else  // adaptive table ���
			strcat(encoding, codeword[pre_ascii][ascii]);	// �ش� ������ codeword�� �ٿ�����

		pre_ascii = ascii;
	}
	max = MAX(prob[pre_ascii]);	// �� ������ ���ڿ� ���� adaptive table �ִ��� Ȯ��
	if (max == 1)	// normal table
		strcat(encoding, codeword_norm[EOD_ascii]);	// normal table�� EOD ����
	else           // adaptive table
		strcat(encoding, codeword[pre_ascii][EOD_ascii]);	// adaptive table�� EOD ����


	int size = strlen(encoding);	// encoding �迭�� ��ü ������ �� 
	int stuffing_num = 8 - (size % 8);	// �� �κ��� 8bit�� ����� ���� 0���� ä�� ��Ʈ�� �� 
	for (int i = 0; i < stuffing_num; i++) {
		if (stuffing_num == 8)		// �� 8bit�� ���� �� ä���������� Ż��
			break;
		encoding[size++] = '0';	// byte align
	}
	encoding[size] = '\0';
}

void make_hbs_file(const char* file_name, char* encoding) {	// 8bit�� ���Ͽ� write�ϴ� �Լ�

	ofstream fout_code(file_name, ios::binary);
	char buffer[9] = { 0, };

	int cnt = 0, num = 0;
	while (encoding[num] != '\0') {
		int index = 0;
		for (num = 8 * cnt; num < 8 * cnt + 8; num++) {	// 8bit�� ����
			buffer[index] = encoding[num];	// 8bit ���ڿ� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�

		int decimal = strtol(buffer, NULL, 2);	// ���ڿ��� 10������ ��ȯ
		char ch = (char)decimal;	// 10������ -127~128�� char������ ��ȯ

		fout_code << ch;	// -127~128�� char���� ���Ͽ� write�ϸ� 16������ �����

		cnt++;
	}
	fout_code.close();
}
