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
	TreeNode* leftChild, * rightChild;	// 왼쪽 자식 노드, 오른쪽 자식 노드

	// 생성자
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
	// 디코딩 
	char* decoding = new char[100000];	// 허프만 디코딩한 문자열을 담는 배열
	memset(decoding, 0, sizeof(char) * 100000);	// 0으로 초기화
	char* decoding_table_norm = new char[1000000];
	memset(decoding_table_norm, 0, sizeof(char) * 1000000);	// 0으로 초기화
	char* decoding_table = new char[1000000];
	memset(decoding_table, 0, sizeof(char) * 1000000);	// 0으로 초기화
	char** codeword_norm = new char* [128];	// 허프만 테이블으로부터 얻은 codeword
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
	char* encoding = new char[100000];	// 16진수로 encoding된 데이터를 2진수로 변환
	memset(encoding, 0, sizeof(char) * 100000);	// 0으로 초기화
	int EOD_ascii = 36;		// 임의로 정한 EOD의 아스키 값('$')
	char EOD[13] = "100101110011";	// EOD의 codeword
	int preceding_symbol = 32;	// adaptive table의 preceding symbol
	TreeNode* root_norm = NULL;					// normal root
	TreeNode* root_preceding[128] = { 0, };		// preceding symbol root

	int pre_ascii=0, ascii=0;


	// huffman_table.hbs로부터 데이터 가져옴
	ifstream table_norm("huffman_table.hbs", ios::in | ios::binary);
	if (!table_norm) {
		printf("error");
		return -1;
	}
	table_norm.seekg(0, table_norm.end);
	int table_len = (int)table_norm.tellg();	// 파일의 크기 구함
	table_norm.seekg(0, table_norm.beg);

	int size = 0;
	int stuffing_num = 0;
	for (int i = 0; i < table_len; i++) {
		char ch;
		table_norm.get(ch);	// 한 문자를 읽어들임
		int decimal = (int)ch;	// 문자를 10진수로 변환

		bitset<8> bs(decimal);	// 10진수를 binary로 변환
		string str = bs.to_string();
		const char* c_str = str.c_str();	// string to const char*
		strcat(decoding_table_norm, c_str);
		size += strlen(c_str);
	}
	table_norm.close();

	// decoding_table_norm로부터 ascii, codeword bit, codeword를 가져옴
	char buffer[30] = { 0, };
	root_norm = new TreeNode();	// 루트 노드 생성
	TreeNode* pCur = NULL;
	
	int cnt = 0, j = 0;
	while(1){
		int index = 0;
		for (j = cnt; j < cnt + 12; j++) {	// EOD 체크
			buffer[index] = decoding_table_norm[j];	// 8bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		if (!strcmp(buffer, EOD))
			break;
		fill_n(buffer, 13, 0);

		index = 0;
		for (j=cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table_norm[j];	// 8bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		ascii = strtol(buffer, NULL, 2);	// 아스키 코드
		fill_n(buffer, 9, 0);
		cnt += 8;
		
		index = 0;
		for (j = cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table_norm[j];	// 8bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		int codeword_bit = strtol(buffer, NULL, 2);	// codeword의 bit 수
		fill_n(buffer, 9, 0);
		cnt += 8;

		index = 0;
		for (j=cnt; j < cnt + codeword_bit; j++) {
			buffer[index] = decoding_table_norm[j];	// codeword 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		strcpy(codeword_norm[ascii], buffer);	// codeword
		char code[30] = { 0, };					
		strcpy(code, buffer);			// codeword
		fill_n(buffer, 30, 0);
		cnt += codeword_bit;
	
		// 허프만 트리 생성
		pCur = root_norm;
		for (int i = 0; i < codeword_bit; i++) {
			if (code[i] == '0') // code의 한 bit가 0이면 왼쪽 하위 노드로 이동
			{
				if (pCur->leftChild == NULL) {
					pCur->leftChild = new TreeNode();
				}

				pCur = pCur->leftChild;
			}
			else if (code[i] == '1')	// code의 한 bit가 1이면 오른쪽 하위 노드로 이동
			{
				if (pCur->rightChild == NULL) {
					pCur->rightChild = new TreeNode();
				}

				pCur = pCur->rightChild;
			}
			else // code의 한 bit가 0이나 1이 아닌 경우 break
			{
				break;
			}
		}

		pCur->ascii = ascii;	// leaf 노드에 아스키 값 저장
	}
	// 허프만 테이블 출력
	//for (int i = 0; i < 128; i++) {
	//	if ((i < 10) || (i > 10 && i < 32) || (i == 127))
	//		continue;
	//	/*if (*codeword_norm[i] == 0)
	//		continue;*/
	//	printf("'%c' : %s\n", i, codeword_norm[i]);
	//}


	// context_adaptive_huffman_table.hbs로부터 데이터 가져옴
	ifstream table("context_adaptive_huffman_table.hbs", ios::in | ios::binary);
	if (!table) {
		printf("error");
		return -1;
	}
	table.seekg(0, table.end);
	table_len = (int)table.tellg();	// 파일의 크기 구함
	table.seekg(0, table.beg);

	size = 0;
	stuffing_num = 0;
	for (int i = 0; i < table_len; i++) {
		char ch;
		table.get(ch);	// 한 문자를 읽어들임
		int decimal = (int)ch;	// 문자를 10진수로 변환

		bitset<8> bs(decimal);	// 10진수를 binary로 변환
		string str = bs.to_string();
		const char* c_str = str.c_str();	// string to const char*
		strcat(decoding_table, c_str);
		size += strlen(c_str);
	}
	table.close();


	// decoding_table로부터 preceding symbol, ascii, codeword bit, codeword를 가져옴
	memset(buffer, 0, sizeof(char) * 30);
	pCur = NULL;

	cnt = 0, j = 0;
	while (1) {
		int index = 0;
		for (j = cnt; j < cnt + 12; j++) {	// EOD 체크
			buffer[index] = decoding_table[j];	// 8bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		if (!strcmp(buffer, EOD))
			break;
		fill_n(buffer, 13, 0);

		index = 0;
		for (j = cnt; j < cnt + 7; j++) {
			buffer[index] = decoding_table[j];	// 7bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		pre_ascii = strtol(buffer, NULL, 2);	// 아스키 코드
		fill_n(buffer, 8, 0);
		cnt += 7;

		index = 0;
		for (j = cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table[j];	// 8bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		ascii = strtol(buffer, NULL, 2);	// 아스키 코드
		fill_n(buffer, 9, 0);
		cnt += 8;

		index = 0;
		for (j = cnt; j < cnt + 8; j++) {
			buffer[index] = decoding_table[j];	// 8bit binary 값 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		int codeword_bit = strtol(buffer, NULL, 2);	// codeword의 bit 수
		fill_n(buffer, 9, 0);
		cnt += 8;

		index = 0;
		for (j = cnt; j < cnt + codeword_bit; j++) {
			buffer[index] = decoding_table[j];	// codeword 저장
			index++;
		}
		buffer[index] = '\0';	// 마지막에 널 문자 추가
		strcpy(codeword[pre_ascii][ascii], buffer);	 // codeword
		char code[30] = { 0, };
		strcpy(code, buffer);						// codeword
		fill_n(buffer, 30, 0);
		cnt += codeword_bit;


		// 허프만 트리 생성
		if(root_preceding[pre_ascii] == NULL)
			root_preceding[pre_ascii] = new TreeNode();	// preceding symbol node
		pCur = root_preceding[pre_ascii];
		for (int i = 0; i < codeword_bit; i++) {
			if (code[i] == '0') // code의 한 bit가 0이면 왼쪽 하위 노드로 이동
			{
				if (pCur->leftChild == NULL) {
					pCur->leftChild = new TreeNode();
				}

				pCur = pCur->leftChild;
			}
			else if (code[i] == '1')	// code의 한 bit가 1이면 오른쪽 하위 노드로 이동
			{
				if (pCur->rightChild == NULL) {
					pCur->rightChild = new TreeNode();
				}

				pCur = pCur->rightChild;
			}
			else // code의 한 bit가 0이나 1이 아닌 경우 break
			{
				break;
			}
		}

		pCur->ascii = ascii;	// leaf 노드에 아스키 값 저장
	}

	//// context adaptive huffman table 출력
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

	
	
	decode("training_input_code.hbs", encoding, root_norm, root_preceding, codeword, decoding, preceding_symbol);	// hbs 파일을 디코딩하여 decoding에 저장
	ofstream output_training("training_output.txt");	// "training_output.txt" 파일에 디코딩 텍스트 출력
	output_training << decoding;
	output_training.close();
	memset(decoding, 0, sizeof(char) * 100000);	// 0으로 초기화
	
	decode("test_input_code.hbs", encoding, root_norm, root_preceding, codeword, decoding, preceding_symbol);	// hbs 파일을 디코딩하여 decoding에 저장
	ofstream output_test("test_output.txt");	// "test_output.txt" 파일에 디코딩 텍스트 출력
	output_test << decoding;
	output_test.close();
	memset(decoding, 0, sizeof(char) * 100000);	// 0으로 초기화



	// 메모리 해제
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
	int EOD_ascii = 36;		// 임의로 정한 EOD의 아스키 값('$')

	ifstream code_training(file_name, ios::in | ios::binary);
	if (!code_training)
		return;
	code_training.seekg(0, code_training.end);
	int code_len = (int)code_training.tellg();	// 파일의 크기 구함
	code_training.seekg(0, code_training.beg);

	// 파일의 16진수 데이터를 binary 데이터로 encoding 배열에 저장 (EOD 포함)
	for (int i = 0; i < code_len; i++) {
		char ch;
		code_training.get(ch);
		int decimal = (int)ch;

		bitset<8> bs(decimal);	// 10진수를 binary로 변환
		string str = bs.to_string();
		const char* c_str = str.c_str();	// string to const char*
		strcat(encoding, c_str);
	}
	memset(buffer, 0, sizeof(char) * 30);	// buffer 배열 비움


	int idx = 0;
	int pre_ascii = 0;
	TreeNode* pCur = root_norm;	// 첫 문자는 normal 허프만 트리에서
	for (int i = 0; i < strlen(encoding); i++) {
		char bit = encoding[i];

		if (bit == '0')		// code의 한 bit가 0이면 왼쪽 하위 노드로 이동
		{
			if (pCur->leftChild == NULL) {
				if (pCur->ascii == EOD_ascii)	// EOD에 도달하면 반복문 탈출
					break;

				decoding[idx] = (char)pCur->ascii;	// decoding 배열에 문자 써넣음
				idx++;

				pre_ascii = pCur->ascii;

				if (pre_ascii == preceding_symbol) {	// pre_ascii가 preceding symbol에 포함되는 경우
					pCur = root_preceding[pre_ascii]->leftChild;	// adaptive 허프만 트리에서 다음 실행
				}
				else {
					pre_ascii = 0;
					pCur = root_norm->leftChild;	// normal 허프만 트리에서 다음 실행
				}
			}
			else {
				pCur = pCur->leftChild;
			}
		}
		else if (bit == '1')	// code의 한 bit가 1이면 오른쪽 하위 노드로 이동
		{
			if (pCur->rightChild == NULL) {
				if (pCur->ascii == EOD_ascii)	// EOD에 도달하면 반복문 탈출
					break;

				decoding[idx] = (char)pCur->ascii;	// decoding 배열에 문자 써넣음
				idx++;

				pre_ascii = pCur->ascii;

				if (pre_ascii == preceding_symbol) {	// pre_ascii가 preceding symbol에 포함되는 경우
					pCur = root_preceding[pre_ascii]->rightChild;	// adaptive 허프만 트리에서 다음 실행
				}
				else {
					pre_ascii = 0;
					pCur = root_norm->rightChild;	// normal 허프만 트리에서 다음 실행
				}
			}
			else {
				pCur = pCur->rightChild;
			}

		}
		else // 0이나 1이 아닌 경우 프로그램 종료
		{
			printf("error!");
			return;
		}
	}
	memset(encoding, 0, sizeof(char) * 100000);	// 0으로 초기화
}