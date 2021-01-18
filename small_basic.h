#ifndef _SMALLBASIC_
#define _SMALLBASIC_

const int NUM_LAB = 100;
const int LAB_LEN = 10;
const int FOR_NEST = 25;
const int SUB_NEST = 25;
const int PROG_SIZE = 10000;

enum tok_types{DELIMITER, VARIABLE, NUMBER, COMMAND, STRING, QUOTE, FUNCTION};
enum tokens {PRINT=1, INPUT, IF, THEN, FOR, NEXT, TO, GOTO, GOSUB, RETURN, EOL, FINISHED, END, SIN, COS, TAN, EXP, LOG, ABS, ATN, SQR};
enum double_ops {LE=1, GE, NE};

extern char *prog;//��������� �� ����� ����� � ���������
extern char *p_buf;//��������� �� ����� ������ ���������
//extern int variables[26]; //�������� 26 ����������� A - Z
extern double variables[26]; //�������� 26 ����������� A - Z

extern struct commands{
	char command[20];
	char tok;
} table[];

extern char token[80];//�������� ��������� ������������� ��������
extern char token_type;//�������� ��� ��������
extern char tok;//�������� ����������������������� ��������


#endif