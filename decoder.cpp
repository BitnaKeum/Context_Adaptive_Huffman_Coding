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
	TreeNode* leftChild, * rightChild;	// ���� �ڽ� ���, ������ �ڽ� ���

	// ������
	TreeNode()
	{
		ascii = 0;
		leftChild = NULL;
		rightChild = NULL;
	}
};

bool exist(int*, int, int);
void decode(const char*, char*, TreeNode*, TreeNode**, char***, char*, int);


int main()
{
	// ���ڵ� 
	char* decoding = new char[100000];	// ������ ���ڵ��� ���ڿ��� ��� �迭
	memset(decoding, 0, sizeof(char) * 100000);	// 0���� �ʱ�ȭ
	char* decoding_table_norm = new char[1000000];
	memset(decoding_table_norm, 0, sizeof(char) * 1000000);	// 0���� �ʱ�ȭ
	char* decoding_table = new char[1000000];
	memset(decoding_table, 0, sizeof(char) * 1000000);	// 0���� �ʱ�ȭ
	char** codeword_norm = new char* [128];	// ������ ���̺����κ��� ���� codeword
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
	char* encoding = new char[100000];	// 16������ encoding�� �����͸� 2������ ��ȯ
	memset(encoding, 0, sizeof(char) * 100000);	// 0���� �ʱ�ȭ
	int EOD_ascii = 36;		// ���Ƿ� ���� EOD�� �ƽ�Ű ��('$')
	char EOD[13] = "100101110011";	// EOD�� codeword
	int preceding_symbol = 32;	// adaptive table�� preceding symbol
	TreeNode* root_norm = NULL;					// normal root
	TreeNode* root_preceding[128] = { 0, };		// preceding symbol root

	int pre_ascii=0, ascii=0;


	// huffman_table.hbs�κ��� ������ ������
	ifstream table_norm("huffman_table.hbs", ios::in | ios::binary);
	if (!table_norm) {
		printf("error");
		return -1;
	}
	table_norm.seekg(0, table_norm.end);
	int table_len = (int)table_norm.tellg();	// ������ ũ�� ����
	table_norm.seekg(0, table_norm.beg);

	int size = 0;
	int stuffing_num = 0;
	for (int i = 0; i < table_len; i++) {
		char ch;
		table_norm.get(ch);	// �� ���ڸ� �о����
		int decimal = (int)ch;	// ���ڸ� 10������ ��ȯ

		bitset<8> bs(decimal);	// 10������ binary�� ��ȯ
		string str = bs.to_string();
		const char* c_str = str.c_str();	// string to const char*
		strcat(decoding_table_norm, c_str);
		size += strlen(c_str);
	}
	table_norm.close();

	// decoding_table_norm�κ��� ascii, codeword bit, codeword�� ������
	char buffer[30] = { 0, };
	root_norm = new TreeNode();	// ��Ʈ ��� ����
	TreeNode* pCur = NULL;
	
	int cnt = 0, j = 0;
	while(1){
		int index = 0;
		for (j = cnt; j < cnt + 12; j++) {	// EOD üũ
			buffer[index] = decoding_table_norm[j];	// 8bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		if (!strcmp(buffer, EOD))
			break;
		fill_n(buffer, 13, 0);

		index = 0;
		for (j=cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table_norm[j];	// 8bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		ascii = strtol(buffer, NULL, 2);	// �ƽ�Ű �ڵ�
		fill_n(buffer, 9, 0);
		cnt += 8;
		
		index = 0;
		for (j = cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table_norm[j];	// 8bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		int codeword_bit = strtol(buffer, NULL, 2);	// codeword�� bit ��
		fill_n(buffer, 9, 0);
		cnt += 8;

		index = 0;
		for (j=cnt; j < cnt + codeword_bit; j++) {
			buffer[index] = decoding_table_norm[j];	// codeword ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		strcpy(codeword_norm[ascii], buffer);	// codeword
		char code[30] = { 0, };					
		strcpy(code, buffer);			// codeword
		fill_n(buffer, 30, 0);
		cnt += codeword_bit;
	
		// ������ Ʈ�� ����
		pCur = root_norm;
		for (int i = 0; i < codeword_bit; i++) {
			if (code[i] == '0') // code�� �� bit�� 0�̸� ���� ���� ���� �̵�
			{
				if (pCur->leftChild == NULL) {
					pCur->leftChild = new TreeNode();
				}

				pCur = pCur->leftChild;
			}
			else if (code[i] == '1')	// code�� �� bit�� 1�̸� ������ ���� ���� �̵�
			{
				if (pCur->rightChild == NULL) {
					pCur->rightChild = new TreeNode();
				}

				pCur = pCur->rightChild;
			}
			else // code�� �� bit�� 0�̳� 1�� �ƴ� ��� break
			{
				break;
			}
		}

		pCur->ascii = ascii;	// leaf ��忡 �ƽ�Ű �� ����
	}
	// ������ ���̺� ���
	//for (int i = 0; i < 128; i++) {
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;
	//	/*if (*codeword_norm[i] == 0)
	//		continue;*/
	//	printf("'%c' : %s\n", i, codeword_norm[i]);
	//}


	// context_adaptive_huffman_table.hbs�κ��� ������ ������
	ifstream table("context_adaptive_huffman_table.hbs", ios::in | ios::binary);
	if (!table) {
		printf("error");
		return -1;
	}
	table.seekg(0, table.end);
	table_len = (int)table.tellg();	// ������ ũ�� ����
	table.seekg(0, table.beg);

	size = 0;
	stuffing_num = 0;
	for (int i = 0; i < table_len; i++) {
		char ch;
		table.get(ch);	// �� ���ڸ� �о����
		int decimal = (int)ch;	// ���ڸ� 10������ ��ȯ

		bitset<8> bs(decimal);	// 10������ binary�� ��ȯ
		string str = bs.to_string();
		const char* c_str = str.c_str();	// string to const char*
		strcat(decoding_table, c_str);
		size += strlen(c_str);
	}
	table.close();


	// decoding_table�κ��� preceding symbol, ascii, codeword bit, codeword�� ������
	memset(buffer, 0, sizeof(char) * 30);
	pCur = NULL;

	cnt = 0, j = 0;
	while (1) {
		int index = 0;
		for (j = cnt; j < cnt + 12; j++) {	// EOD üũ
			buffer[index] = decoding_table[j];	// 8bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		if (!strcmp(buffer, EOD))
			break;
		fill_n(buffer, 13, 0);

		index = 0;
		for (j = cnt; j < cnt + 7; j++) {
			buffer[index] = decoding_table[j];	// 7bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		pre_ascii = strtol(buffer, NULL, 2);	// �ƽ�Ű �ڵ�
		fill_n(buffer, 8, 0);
		cnt += 7;

		index = 0;
		for (j = cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table[j];	// 8bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		ascii = strtol(buffer, NULL, 2);	// �ƽ�Ű �ڵ�
		fill_n(buffer, 9, 0);
		cnt += 8;

		index = 0;
		for (j = cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table[j];	// 8bit binary �� ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		int codeword_bit = strtol(buffer, NULL, 2);	// codeword�� bit ��
		fill_n(buffer, 9, 0);
		cnt += 8;

		index = 0;
		for (j = cnt; j < cnt + codeword_bit; j++) {
			buffer[index] = decoding_table[j];	// codeword ����
			index++;
		}
		buffer[index] = '\0';	// �������� �� ���� �߰�
		strcpy(codeword[pre_ascii][ascii], buffer);	 // codeword
		char code[30] = { 0, };
		strcpy(code, buffer);						// codeword
		fill_n(buffer, 30, 0);
		cnt += codeword_bit;


		// ������ Ʈ�� ����
		if(root_preceding[pre_ascii] == NULL)
			root_preceding[pre_ascii] = new TreeNode();	// preceding symbol node
		pCur = root_preceding[pre_ascii];
		for (int i = 0; i < codeword_bit; i++) {
			if (code[i] == '0') // code�� �� bit�� 0�̸� ���� ���� ���� �̵�
			{
				if (pCur->leftChild == NULL) {
					pCur->leftChild = new TreeNode();
				}

				pCur = pCur->leftChild;
			}
			else if (code[i] == '1')	// code�� �� bit�� 1�̸� ������ ���� ���� �̵�
			{
				if (pCur->rightChild == NULL) {
					pCur->rightChild = new TreeNode();
				}

				pCur = pCur->rightChild;
			}
			else // code�� �� bit�� 0�̳� 1�� �ƴ� ��� break
			{
				break;
			}
		}

		pCur->ascii = ascii;	// leaf ��忡 �ƽ�Ű �� ����
	}

	//// context adaptive huffman table ���
	//for (int i = 0; i < 128; i++) {	// preceding symbol
	//	//if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//	//	continue;
	//	for (int j = 0; j < 128; j++) {	// symbol
	//		//if ((j < 10) || (j > 10 && j < 32) || (j == 127))
	//		//	continue;
	//		if(*codeword[i][j] == 0)
	//			continue;
	//		printf("'%c''%c'\t\t%s\n", i, j, codeword[i][j]);	// symbol, probability, codeword
	//	}
	//	printf("\n");
	//}

	
	
	decode("training_input_code.hbs", encoding, root_norm, root_preceding, codeword, decoding, preceding_symbol);	// hbs ������ ���ڵ��Ͽ� decoding�� ����
	ofstream output_training("training_output.txt");	// "training_output.txt" ���Ͽ� ���ڵ� �ؽ�Ʈ ���
	output_training << decoding;
	output_training.close();
	memset(decoding, 0, sizeof(char) * 100000);	// 0���� �ʱ�ȭ
	
	decode("test_input_code.hbs", encoding, root_norm, root_preceding, codeword, decoding, preceding_symbol);	// hbs ������ ���ڵ��Ͽ� decoding�� ����
	ofstream output_test("test_output.txt");	// "test_output.txt" ���Ͽ� ���ڵ� �ؽ�Ʈ ���
	output_test << decoding;
	output_test.close();
	memset(decoding, 0, sizeof(char) * 100000);	// 0���� �ʱ�ȭ



	// �޸� ����
	delete[] decoding;
	delete[] decoding_table_norm;
	delete[] decoding_table;
	delete[] encoding;
	for (int i = 0; i < 128; i++)
		delete[] codeword[i];
	delete[] codeword;
}

//bool exist(int* arr, int size, int find_value)
//{
//	for (int i = 0; i < size; i++) {
//		if (arr[i] == find_value)
//			return true;
//	}
//
//	return false;
//}

void decode(const char* file_name, char* encoding, TreeNode* root_norm, TreeNode** root_preceding, char*** codeword, char* decoding, int preceding_symbol)
{
	char buffer[30] = { 0, };
	int EOD_ascii = 36;		// ���Ƿ� ���� EOD�� �ƽ�Ű ��('$')

	ifstream code_training(file_name, ios::in | ios::binary);
	if (!code_training)
		return;
	code_training.seekg(0, code_training.end);
	int code_len = (int)code_training.tellg();	// ������ ũ�� ����
	code_training.seekg(0, code_training.beg);

	// ������ 16���� �����͸� binary �����ͷ� encoding �迭�� ���� (EOD ����)
	for (int i = 0; i < code_len; i++) {
		char ch;
		code_training.get(ch);
		int decimal = (int)ch;

		bitset<8> bs(decimal);	// 10������ binary�� ��ȯ
		string str = bs.to_string();
		const char* c_str = str.c_str();	// string to const char*
		strcat(encoding, c_str);
	}
	memset(buffer, 0, sizeof(char) * 30);	// buffer �迭 ���


	int idx = 0;
	int pre_ascii = 0;
	TreeNode* pCur = root_norm;	// ù ���ڴ� normal ������ Ʈ������
	for (int i = 0; i < strlen(encoding); i++) {
		char bit = encoding[i];

		if (bit == '0')		// code�� �� bit�� 0�̸� ���� ���� ���� �̵�
		{
			if (pCur->leftChild == NULL) {
				if (pCur->ascii == EOD_ascii)	// EOD�� �����ϸ� �ݺ��� Ż��
					break;

				decoding[idx] = (char)pCur->ascii;	// decoding �迭�� ���� �����
				idx++;

				pre_ascii = pCur->ascii;

				if (pre_ascii == preceding_symbol) {	// pre_ascii�� preceding symbol�� ���ԵǴ� ���
					pCur = root_preceding[pre_ascii]->leftChild;	// adaptive ������ Ʈ������ ���� ����
				}
				else {
					pre_ascii = 0;
					pCur = root_norm->leftChild;	// normal ������ Ʈ������ ���� ����
				}
			}
			else {
				pCur = pCur->leftChild;
			}
		}
		else if (bit == '1')	// code�� �� bit�� 1�̸� ������ ���� ���� �̵�
		{
			if (pCur->rightChild == NULL) {
				if (pCur->ascii == EOD_ascii)	// EOD�� �����ϸ� �ݺ��� Ż��
					break;

				decoding[idx] = (char)pCur->ascii;	// decoding �迭�� ���� �����
				idx++;

				pre_ascii = pCur->ascii;

				if (pre_ascii == preceding_symbol) {	// pre_ascii�� preceding symbol�� ���ԵǴ� ���
					pCur = root_preceding[pre_ascii]->rightChild;	// adaptive ������ Ʈ������ ���� ����
				}
				else {
					pre_ascii = 0;
					pCur = root_norm->rightChild;	// normal ������ Ʈ������ ���� ����
				}
			}
			else {
				pCur = pCur->rightChild;
			}

		}
		else // 0�̳� 1�� �ƴ� ��� ���α׷� ����
		{
			printf("error!");
			return;
		}
	}
	memset(encoding, 0, sizeof(char) * 100000);	// 0���� �ʱ�ȭ
}