#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "test.txt"
#define MAX_SIZE_STACK 100
#define SIZE    512
#define MEMSIZE 100
#define REGSIZE 10
#define ����    00
#define ����    02
#define �ƴϸ�  03
#define �ݺ�    04
#define ���    05
#define ��      06
#define ����    07
#define TRUE    1
#define FALSE   0

typedef struct {
	int stack[MAX_SIZE_STACK];
	int top;
}StackType;

typedef enum {
	INT, STR
} TYPE;

typedef struct variable {
	char var[SIZE];	// ���� �̸�
	TYPE type;	// Ÿ���� ���� ������ �ʵ�

	union {
		int intVar;	// �������� ��� ����� ����
		char charVar[SIZE];	// ���ڿ��� ��� ����� ����
	} value;
} VARIABLE;

int init(char[][SIZE], char[][SIZE], VARIABLE[]);
void fetch(char[][SIZE], char[][SIZE], int *);
int lexer(char[][SIZE], char[][SIZE]);
void interpreter(char[][SIZE], char[][SIZE], VARIABLE[], int);
void id(char R[][SIZE], VARIABLE symboltable[], VARIABLE tmpStructVal);
void start(char[][SIZE], int, int);
void stmt(int, char[][SIZE], char[][SIZE], VARIABLE[], int *, int);
void assign_stmt(char[][SIZE], VARIABLE[], int);
int if_stmt(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine);
void else_stmt(char[][SIZE], char[][SIZE], int *, int, int);
void loop_stmt(char[][SIZE], char[][SIZE], VARIABLE[], int *, int);
void output(char[][SIZE], VARIABLE[], int);
int expr(char R[][SIZE], VARIABLE symboltable[], int listCnt, char tmpVar[], char expression[]);
int conditional_expr(char M[][SIZE], char condStmt[], VARIABLE symboltable[], int *ProgramCounter, int opLine);
void eliminate(char *str, char ch);

void initStack(StackType *);
int is_empty(StackType *);
int is_full(StackType *);
void push(StackType *, int);
int pop(StackType *);
int peek(StackType *);
int prec(char);
void infix_to_postfix(char[], const char[]);
int eval(char[]);

char delmt[] = " ,\t\n(){.";
int listCnt = 0;

/* Main �Լ� */
int main(void) {
	char M[MEMSIZE][SIZE] = { 0, }, R[REGSIZE][SIZE] = { {0, }, };	// �޸𸮿� �������� �뵵�� �������迭(���ڿ��迭)�� ����
	VARIABLE symboltable[SIZE];	// �ɺ����̺� ����
	int opLine = 0;
	memset(symboltable, 0, sizeof(symboltable));
	opLine = init(M, R, symboltable);	// ���α׷��־�� �� ������ ���� �ۼ��� ���α׷� �ҽ��ڵ��� ������ ��� �о�鿩 M�� �����ϴ� �Լ� ȣ��

	while (1) {
		interpreter(M, R, symboltable, opLine);	// ��ɾ���� �� �྿ ������ �ҽ��ڵ带 �ؼ��� ���� �ϴ� ���������� �Լ� ȣ��
	}

	return 0;
}

/* ������ ���� ���α׷��־� ������ ���� ����� �ҽ��ڵ� ��ü�� M�̶�� ���ڿ� �迭�� ���� �����ϴ� �Լ�(HDD to MM) */
int init(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[]) {
	int i = 0;
	char file[SIZE] = FILENAME;
	FILE *fSrc = NULL;
	errno_t err;

	if ((err = fopen_s(&fSrc, file, "r")) != 0) {
		fprintf(stderr, "%s\n", "���� ���� ����");
		exit(1);
	}

	while (!feof(fSrc)) {
		fgets(M[i], SIZE, fSrc);
		i++;
	}

	fclose(fSrc);
	return i;
}

/* M���ڿ��迭(�޸�)���� R���ڿ��迭(��������)�� �ѹ��徿 fetch�ϴ� ����� �Լ� (MM to Register) � ��ɾ���� �о������� ���α׷�ī���Ͱ� ����ϰ� �ִ� */
void fetch(char M[][SIZE], char R[][SIZE], int *ProgramCounter) {
	strcpy_s(R[0], SIZE, M[*ProgramCounter]);
	(*ProgramCounter)++;
}

/* ����������_���پ� �޸��� ��ɾ���� �������ͷ� �ű� ��(fetch) lexer���ֺм��⸦ ���� ��ɾ��� opcode(��ū)�� �ص��ϰ� ���͹̳ο� �´� procedure�� �����ϵ��� ��ȯ�޾Ƽ� ������ �����Ѵ�. */
void interpreter(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int opLine) {
	static int ProgramCounter = 0;
	int opcode = -1;

	fetch(M, R, &ProgramCounter);
	opcode = lexer(M, R);
	stmt(opcode, M, R, symboltable, &ProgramCounter, opLine);	// � ���͹̳��� ���°����� stmt�� �Բ� �������ش�
}

/* �������Ϳ� ����ִ� ù��° ���ڿ��� oper�迭�� �����ؿ� delmt�� �������� �ڸ� ��
* ��е� ���ڿ��� �ι�° ���ڿ��� context�� �����ϰ� ù��° ���ڿ��� ��ȯ�޾� lexeme�� �����Ѵ�.
* ù ��° ���ڿ� lexeme�� ���� ���������� ���� opcode�� ��ȯ���ش�. ���͹̳��� �����ϴ� ������ �Լ� */
int lexer(char M[][SIZE], char R[][SIZE]) {
	char delmt[] = " ,\t\n(){}.";
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };

	strcpy_s(oper, SIZE, R[0]);

	lexeme = strtok_s(oper, delmt, &context);

	if (lexeme != NULL) {
		if (strcmp(lexeme, "����") == 0)   return ����;
		if (strcmp(lexeme, "����") == 0)   return ����;
		if (strcmp(lexeme, "�ƴϸ�") == 0) return �ƴϸ�;
		if (strcmp(lexeme, "�ݺ�") == 0)   return �ݺ�;
		if (strcmp(lexeme, "���") == 0)   return ���;
		if (strcmp(lexeme, "��") == 0)     return ��;

		return ����;
	}
	return -1;
}

/* <stmt> ���͹̳ο� ���� procedure ����
* <stmt> ::= <expr> | <assign_stmt> | <if_stmt> | <loop_stmt> | <output>
* �� ���� ���õǾ������� ���� �� ���͹̳ο� �����Ǵ� procedure�� ȣ���ϰ� �� */
void stmt(int opcode, char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	static int logicRes = FALSE;

	// �����ؿ� opcode�� � ���͹̳��� ���õǾ������� ���� �׿� �ش��ϴ� procedure�� ȣ���Ѵ�.
	switch (opcode) {
	case ����:	// ���� ��ȣ�� ���� ó��
		start(R, *ProgramCounter, opLine);
		break;
	case ����:	// <if_stmt>
		logicRes = if_stmt(M, R, symboltable, ProgramCounter, opLine);
		break;
	case �ƴϸ�:	// <else_stmt>
		else_stmt(M, R, ProgramCounter, opLine, logicRes);
		break;
	case �ݺ�:	// <loop_stmt>
		loop_stmt(M, R, symboltable, ProgramCounter, opLine);
		break;
	case ���:	// <output>
		output(R, symboltable, listCnt);
		break;
	case ��:	// �� ��ȣ�� ���� ó��
		_getch();
		exit(1);
		break;
	case ����:	// <assign_stmt>
		assign_stmt(R, symboltable, listCnt);
		break;
	}
}

/* ���۱�ȣ�� �ش� ���� ������ �о��°��� ����ϴ� ���� ProgramCounter ������ ������������ ����ȣ�� ���������ϸ� ���α׷��� ����������� �������Ƿ� error �� throw�Ѵ� */
void start(char R[][SIZE], int ProgramCounter, int opLine) {
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };

	strcpy_s(oper, SIZE, R[0]);

	lexeme = strtok_s(oper, delmt, &context);
	if (lexeme != NULL) {
		if (ProgramCounter > opLine) {
			fprintf(stderr, "%s\n", "Error");
			exit(1);
		}
	}
}

/* ���͹̳� <id>�� ���� procedure�μ�
* ���α׷��־��� ����Ÿ������ �����ϹǷ�
* ������ ���� ������ ������ �Ҵ繮���� ȣ��� �Լ��̴�.
* symboltable�� ������ �������ִ� ������ �Լ� */
void id(char R[][SIZE], VARIABLE symboltable[], VARIABLE tmpStructVal) {
	symboltable[listCnt] = tmpStructVal;	// ���� ���� VARIABLE�� symboltable�� ����
	listCnt++;	// symboltable�� ��� ������ ����Ǿ��ִ��� ����ϴ� �뵵�� ��������
}

/* ���͹̳� <if_stmt>�� �ش��ϴ� procedure�μ�
* conditional_expr�� üũ�� ���� if�� Ȥ�� else���� ����ǰ� �ȴ�. */
int if_stmt(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	char oper[SIZE] = { 0, };
	char *tmpLex = NULL;
	char *context = NULL;

	if (conditional_expr(M, R[0], symboltable, ProgramCounter, opLine)) {	// ���ǽ��� ���ΰ��
		(*ProgramCounter)++;
		while ((*ProgramCounter) < opLine) {
			strcpy_s(oper, SIZE, M[(*ProgramCounter)]);
			tmpLex = strtok_s(oper, delmt, &context);
			if (strcmp(tmpLex, "}") == 0) {
				return FALSE;
			}
			(*ProgramCounter)++;
		}
	}
	else {
		return TRUE;
	}
}

/* ���͹̳� <cond_expr>�� �ش��ϴ� procedure�μ� ���ǽĿ� ���� TRUE of FALSE ���� ��ȯ�ϴ� �Լ� */
int conditional_expr(char M[][SIZE], char condStmt[], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };
	char logicOper[SIZE] = { 0, };
	char *tmpLex = NULL;
	int i = 0;
	int originInt = 0, targetInt = 0;
	char originStr[SIZE] = { 0, };
	char targetStr[SIZE] = { 0, };
	TYPE firstOperandType = INT;

	strcpy_s(oper, SIZE, condStmt);

	lexeme = strtok_s(oper, delmt, &context);	// ��ȣ ������ ����
	lexeme = strtok_s(NULL, delmt, &context);	// ������ ��Ƶα�

	// ���� ������ ������ �����Ѵٸ� �̸� ���� ���� �����صд�
	while (i < listCnt) {
		if (strcmp(lexeme, symboltable[i].var) == 0) {
			if (symboltable[i].type == INT) {
				originInt = symboltable[i].value.intVar;
				firstOperandType = INT;
			}
			else if (symboltable[i].type == STR) {
				strcpy_s(originStr, SIZE, symboltable[i].value.charVar);
				firstOperandType = STR;
			}
			break;
		}
		i++;
	}

	// �񱳿����� �����صα�
	lexeme = strtok_s(NULL, delmt, &context);
	strcpy_s(logicOper, SIZE, lexeme);

	// �ǿ����� Ȯ��
	lexeme = strtok_s(NULL, delmt, &context);
	if (atoi(lexeme) == 0) {	// �ǿ����ڰ� ������ ���
		i = 0;
		// ���� ������ ������ �����Ѵٸ� �̸� Ÿ�� ���� �����صд�
		while (i < listCnt) {
			if (strcmp(lexeme, symboltable[i].var) == 0) {
				if ((symboltable[i].type == INT) && (firstOperandType == INT)) {	// ù��° �ǿ����ڿ� Ÿ���� ������ ���ٸ�
					targetInt = symboltable[i].value.intVar;
				}
				else if ((symboltable[i].type == STR) && (firstOperandType == STR)) {	// ù��° �ǿ����ڿ� Ÿ���� ���ڿ��� ���ٸ�
					strcpy_s(targetStr, SIZE, symboltable[i].value.charVar);
				}
				else {
					printf("������ ���ڿ��� �񱳴� �Ұ��մϴ�.");
					return -1;
				}
				break;
			}
			i++;
		}
	}
	else if (lexeme[0] == '\"') {
		if (firstOperandType == STR) {
			eliminate(lexeme, '\"');
			strcpy_s(targetStr, SIZE, lexeme);
		}
		else {
			printf("������ ���ڿ��� �񱳴� �Ұ��մϴ�.");
			return -1;	//�̷����ص��ǳ�
		}
	}
	else {	// �ǿ����ڰ� ���ڶ��
		if (firstOperandType == INT) {
			targetInt = atoi(lexeme);
		}
		else {
			printf("������ ���ڿ��� �񱳴� �Ұ��մϴ�.");
			return -1;	//�̷����ص��ǳ�
		}
	}

	// ���������� ���� ���
	if (firstOperandType == INT) {
		if (strcmp(logicOper, ">") == 0) {
			return (!(originInt > targetInt));
		}
		if (strcmp(logicOper, "<") == 0) {
			return (!(originInt < targetInt));
		}
		if (strcmp(logicOper, ">=") == 0) {
			return (!(originInt >= targetInt));
		}
		if (strcmp(logicOper, "<=") == 0) {
			return (!(originInt <= targetInt));
		}
		if (strcmp(logicOper, "==") == 0) {
			return (!(originInt == targetInt));
		}
		if (strcmp(logicOper, "!=") == 0) {
			return (!(originInt != targetInt));
		}
	}
	// ���ڿ������� ���� ���
	else if (firstOperandType == STR) {
		int result = strcmp(originStr, targetStr);
		if (strcmp(logicOper, ">") == 0) {
			return (!(result > 0));
		}
		if (strcmp(logicOper, "<") == 0) {
			return (!(result < 0));
		}
		if (strcmp(logicOper, "==") == 0) {
			return (!(result == 0));
		}
		if (strcmp(logicOper, "!=") == 0) {
			return (!(result != 0));
		}
	}

	return -1;
}

/* ���͹̳η� ���������� ������ �ǵ����� ���Ͽ� �Լ��� �и��Ͽ���.
* conditional_expr�� üũ�� ���� if�� false��ȯ�� else���� ����ǰ� �ȴ�. */
void else_stmt(char M[][SIZE], char R[][SIZE], int *ProgramCounter, int opLine, int logicRes) {
	char *tmpLex = NULL;
	char *context = NULL;

	if (logicRes == TRUE) {
		(*ProgramCounter)++;
		while ((*ProgramCounter) < opLine) {
			tmpLex = strtok_s(M[(*ProgramCounter)], delmt, &context);
			if (strcmp(tmpLex, "}") == 0) {
				(*ProgramCounter)++;
				return;
			}
			(*ProgramCounter)++;
		}
	}
}

/* ���͹̳� <loop_stmt>�� �ش��ϴ� procedure�μ�
* conditional_expr�� ��ȯ���� TRUE�� ��� ��� �� �ڵ���� �ݺ������� ����ǰ� �ȴ�. */
void loop_stmt(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };
	int i = 0, limit = 0;
	int tmpProgramCounter = *ProgramCounter;
	int opcode = -1;

	strcpy_s(oper, SIZE, R[0]);

	char condStmt[SIZE] = { 0, };
	strcpy_s(condStmt, SIZE, R[0]);	// �ݺ����ǹ� ��Ƴ���
	lexeme = strtok_s(oper, delmt, &context);	// ��ȣ ����
	lexeme = strtok_s(NULL, delmt, &context);	// ���ǽĺκи� ��������

	while (!(conditional_expr(M, condStmt, symboltable, ProgramCounter, opLine))) {	// �Ź� ���ǹ��� Ȯ���ϵ���
		fetch(M, R, ProgramCounter);
		strcpy_s(oper, SIZE, R[0]);
		lexeme = strtok_s(oper, delmt, &context);
		if (strcmp(lexeme, "}") == 0) {
			(*ProgramCounter) = tmpProgramCounter;
			continue;
		}
		opcode = lexer(M, R);
		stmt(opcode, M, R, symboltable, ProgramCounter, opLine);
	}
}

/* ���͹̳� <output>�� �ش��ϴ� procedure�� type�� ���� ����� �˸°� formating�Ͽ� ����Ѵ� */
void output(char R[][SIZE], VARIABLE symboltable[], int listCnt) {
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };
	int i = 0;

	strcpy_s(oper, SIZE, R[0]);

	lexeme = strtok_s(oper, delmt, &context);
	lexeme = strtok_s(NULL, delmt, &context);

	while (i < listCnt) {
		if (strcmp(lexeme, symboltable[i].var) == 0) {
			if (symboltable[i].type == INT) {
				printf("(������ ����) %s: %d\n", symboltable[i].var, symboltable[i].value.intVar);
			}
			else if (symboltable[i].type == STR) {
				printf("(���ڿ��� ����) %s: %s\n", symboltable[i].var, symboltable[i].value.charVar);
			}

			break;
		}
		i++;
	}

	if (i >= listCnt) {
		printf("���ǵ��� ���� ���� ��� ����\n");
	}
}

/* ���͹̳� <assign_stmt>�� �ش��ϴ� procedure��, id�� expr�� ����� �Ҵ��Ѵ�. <expr>�κп����� ���� Ÿ�Կ� �°� �򰡵ȴ� */
void assign_stmt(char R[][SIZE], VARIABLE symboltable[], int listCnt) {
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };
	int i = 0, tmpValue = 0, len = 0;
	char tmpVar[SIZE] = { 0, };
	char tmpOper[SIZE] = { 0, };
	char postfix[SIZE] = { 0, };
	char *tmpLex = NULL;
	int isStr = FALSE;
	strcpy_s(oper, SIZE, R[0]);	// R�迭�� ����Ǿ��ִ� ������ ������ oper�� ����ǰ�

	lexeme = strtok_s(oper, delmt, &context);	// �� ���� ��ü�� delmt�� �������� ����Ͽ� ù��° ���ڿ��� lexeme��, �ι�° ���ڿ��� context�� �������ش�.
	strcpy_s(tmpVar, SIZE, lexeme);	// lexeme�� tmpVar�� �����س���

	lexeme = strtok_s(NULL, delmt, &context);	// ���� ��ū��
	if (strcmp(lexeme, "=") == 0) {	// "="���������� Ȯ���Ͽ�
		if (!(expr(R, symboltable, listCnt, tmpVar, context))) {	// ���������� ���� ����� �Ϸ���� �ʾѴٸ�
			printf("�߸��� �������� ���� �Ҵ繮 ����\n");
		}
	}
	else {
		printf("error �ùٸ� ������ �ƴմϴ�.\n");
	}
}

/* ���͹̳� <expr>�� �ش��ϴ� procedure�μ� ��(������,�ǿ�����,Ÿ�� ��)�� ���ϰ� �������� ���� �������θ� ��ȯ�Ѵ� */
int expr(char R[][SIZE], VARIABLE symboltable[], int listCnt, char tmpVar[], char expression[]) {
	char *lexeme = NULL;
	char *context = NULL;
	int i = 0, tmpValue = 0, len = 0;
	char tmpOper[SIZE] = { 0, };
	char postfix[SIZE] = { 0, };
	char *tmpLex = NULL;
	int isStr = FALSE;

	//ö��\0= ö�� + 1 * 3, ö�� + 1 * 3�� expression�� �������
	lexeme = strtok_s(expression, delmt, &context);	// ���� ��ū�� lexeme�� ���

	if (lexeme[0] == '\"') {	// ���� ��ū�� ù ���ڰ� �����ڿ�Ʈ�� �����ϴ� ���
		/*
		while () {	// �ѹ��� �� ������ ���� �ݺ�
			// �ǿ����ڰ� ������ error
			// �����ڴ� +�� ����
			// �ǿ����ڰ� ""�ΰ�� strcat()
			// �ǿ����ڰ� �����ΰ�� ������ Ÿ���� str�� ������ ������ ���� strcat()
		}
		*/
		char tmpStr[SIZE] = { 0, };
		int flag = TRUE;	// ���ڿ� ������ �ϴ� �Ϳ� �־ +�����ڰ� �����߾��ٴ� ǥ�ø� �ϱ����� �÷��� ����

		//strcpy_s(tmpStr, SIZE, lexeme);	// �ϴ� �� ���ڿ����ͷ��� �ӽ� ���ڹ迭�� �����س��´�
		while (lexeme != NULL) {	// �ش� ������ ����������
			if (strcmp(lexeme, "+") == 0) {
				flag = TRUE;
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if ((lexeme[0] == '\"') && (flag)) {	// ���ο� ���ڿ� ���ͷ��� �����߰�, �ռ��� +�����ڵ� �־��ٸ�
				strcat_s(tmpStr, SIZE, lexeme);	// tmpStr �ڿ� lexeme�� �̾���δ�.
				flag = FALSE;	// ���׿������� +�� �ǿ����ڸ� �ٽ�ٴ°��� �ǹ���
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if ((atoi(lexeme) == 0) && (flag)) {	// ������ �����߰�, �ռ��� +�����ڵ� �־��ٸ�
				i = 0;
				while (i < listCnt) {	// �ش� ������ �����ϴ� �������� Ȯ��
					if (strcmp(lexeme, symboltable[i].var) == 0) {	// ���� �̸��� ������ �ɺ����̺� �ִٸ�
						if (symboltable[i].type == STR) {	// ���ڿ� �������
							strcat_s(tmpStr, SIZE, symboltable[i].value.charVar);	// �ش� ������ ���� ���� tmpStr �ڿ� �̾���δ�.
						}
						else {
							return -1;	// error Ÿ���� ���� ���� �߸��� ����
						}
						break;
					}
					i++;
				}

				if (i >= listCnt) {	// �ش� ������ �ɺ����̺� �����ٸ�
					return -1;	// ���ǵ��� ���� ���� �̿��� ����
				}
				flag = FALSE;	// ���׿������� +�� �ǿ����ڸ� �ٽ�ٴ°��� �ǹ���
			}
			else {	// �� �� �����ڸ� ����ߴٰų�, �������� ���Ϸ����ߴٰų� �ϴ� ��쿣
				return -1;	// ���� �߻�-�߸��� ���� �ǿ����ڰ��� Ÿ�Կ���
			}
			lexeme = strtok_s(NULL, delmt, &context);
		}

		i = 0;
		while (i < listCnt) {	// �Ҵ繮�� ���� �����̸��� ������ �����ߴٸ� �ߺ� ������ ���� �ƴ϶� �� ������ Ÿ�԰� �� ���� �����϶�.
			if (strcmp(tmpVar, symboltable[i].var) == 0) {
				eliminate(tmpStr, '\"');
				strcpy_s(symboltable[i].value.charVar, SIZE, tmpStr);	// �ش� ������ ���� tmpStr�� �������ش�.
				symboltable[i].type = STR;
				break;
			}
			i++;
		}
		if (i >= listCnt) {	// �����̸��� �ɺ����̺��� ã�� ���ߴٸ� ��, �űԺ������
			VARIABLE tmpStructVal;
			strcpy_s(tmpStructVal.var, SIZE, tmpVar);	// �����̸� �����ش�� �����ϰ�
			tmpStructVal.type = STR;	// ���� Ÿ���� STR�� �����ϰ�
			eliminate(tmpStr, '\"');
			strcpy_s(tmpStructVal.value.charVar, SIZE, tmpStr);	// �ش� ������ ���� tmpStr�� �������ְ�
			id(R, symboltable, tmpStructVal);	// �ĺ���(����) �����϶�� �Ѱ��ش�.
			//printf("listCnt: %d\n", listCnt);
		}
	}
	else if (atoi(lexeme) == 0) {	// ���� ��ū�� �������� ���
		/*�� ������ Ÿ���� �м��� ���� ��(���ڿ� ���� ����) �Ʒ�(���� ���� ����) �������ָ� �� */
		i = 0;
		while (i < listCnt) {	// �ش� ������ �����ϴ� �������� Ȯ��
			if (strcmp(lexeme, symboltable[i].var) == 0) {	// ���� �̸��� ������ �ɺ����̺� �ִٸ�
				if (symboltable[i].type == STR) {	// ���ڿ� �������
					char tmpStr[SIZE] = { 0, };
					int flag = FALSE;	// ���ڿ� ������ �ϴ� �Ϳ� �־ +�����ڰ� �����߾��ٴ� ǥ�ø� �ϱ����� �÷��� ����

					strcpy_s(tmpStr, SIZE, symboltable[i].value.charVar);	// �ش� ������ ���� ���� �ӽ� ���ڹ迭�� �����س��´�
					lexeme = strtok_s(NULL, delmt, &context);

					while (lexeme != NULL) {	// �ش� ������ ����������
						if (strcmp(lexeme, "+") == 0) {
							flag = TRUE;
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if ((lexeme[0] == '\"') && (flag)) {	// ���ο� ���ڿ� ���ͷ��� �����߰�, �ռ��� +�����ڵ� �־��ٸ�
							strcat_s(tmpStr, SIZE, lexeme);	// tmpStr �ڿ� lexeme�� �̾���δ�.
							flag = FALSE;	// ���׿������� +�� �ǿ����ڸ� �ٽ�ٴ°��� �ǹ���
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if ((atoi(lexeme) == 0) && (flag)) {	// ������ �����߰�, �ռ��� +�����ڵ� �־��ٸ�
							i = 0;
							while (i < listCnt) {	// �ش� ������ �����ϴ� �������� Ȯ��
								if (strcmp(lexeme, symboltable[i].var) == 0) {	// ���� �̸��� ������ �ɺ����̺� �ִٸ�
									if (symboltable[i].type == STR) {	// ���ڿ� �������
										strcat_s(tmpStr, SIZE, symboltable[i].value.charVar);	// �ش� ������ ���� ���� tmpStr �ڿ� �̾���δ�.
									}
									else {
										return -1;	// error Ÿ���� ���� ���� �߸��� ����
									}
									break;
								}
								i++;
							}

							if (i >= listCnt) {	// �ش� ������ �ɺ����̺� �����ٸ�
								return -1;	// ���ǵ��� ���� ���� �̿��� ����
							}
							flag = FALSE;	// ���׿������� +�� �ǿ����ڸ� �ٽ�ٴ°��� �ǹ���
						}
						else {	// �� �� �����ڸ� ����ߴٰų�, �������� ���Ϸ����ߴٰų� �ϴ� ��쿣
							return -1;	// ���� �߻�-�߸��� ���� �ǿ����ڰ��� Ÿ�Կ���
						}
						lexeme = strtok_s(NULL, delmt, &context);
					}
					i = 0;
					while (i < listCnt) {	// �Ҵ繮�� ���� �����̸��� ������ �����ߴٸ� Ÿ�԰� �� ���� �����϶�. (���� ������ Ÿ���� ����_����Ÿ�����̴ϱ�)
						if (strcmp(tmpVar, symboltable[i].var) == 0) {
							eliminate(tmpStr, '\"');
							strcpy_s(symboltable[i].value.charVar, SIZE, tmpStr);	// �ش� ������ ���� tmpStr�� �������ش�.
							symboltable[i].type = STR;
							break;
						}
						i++;
					}
					if (i >= listCnt) {	// �����̸��� �ɺ����̺��� ã�� ���ߴٸ� ��, �űԺ������
						VARIABLE tmpStructVal;
						strcpy_s(tmpStructVal.var, SIZE, tmpVar);	// �����̸� �����ش�� �����ϰ�
						tmpStructVal.type = STR;	// ���� Ÿ���� STR�� �����ϰ�
						eliminate(tmpStr, '\"');
						strcpy_s(tmpStructVal.value.charVar, SIZE, tmpStr);	// �ش� ������ ���� tmpStr�� �������ְ�
						id(R, symboltable, tmpStructVal);	// �ĺ���(����) �����϶�� �Ѱ��ش�.
						//printf("listCnt: %d\n", listCnt);
					}
				}
				else if (symboltable[i].type == INT) {	//	�������̶��
					tmpOper[len++] = 48 + symboltable[i].value.intVar;	// �ϴ� �� ���� �迭���ÿ� �־����
					lexeme = strtok_s(NULL, delmt, &context);
					while (lexeme != NULL) {
						if (strcmp(lexeme, "+") == 0) {
							tmpOper[len++] = '+';
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if (strcmp(lexeme, "-") == 0) {
							tmpOper[len++] = '-';
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if (strcmp(lexeme, "*") == 0) {
							tmpOper[len++] = '*';
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if (strcmp(lexeme, "/") == 0) {
							tmpOper[len++] = '/';
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if (atoi(lexeme) == 0) {	// �������
							i = 0;
							while (i < listCnt) {
								if (strcmp(lexeme, symboltable[i].var) == 0) {
									if (symboltable[i].type == INT) {	// ������ �������
										tmpOper[len++] = 48 + symboltable[i].value.intVar;	// �ش� ������ �������� �迭���ÿ� ����ش�.
									}
									else {	// �׿��� Ÿ����
										return -1;	// error-�߸��� ����
									}
								}
								i++;
							}

							if (i >= listCnt) {	// �ش� ������ �ɺ����̺� �����ٸ�
								return -1;	// ���ǵ��� ���� ���� �̿��� ����
							}
						}
						else {	// ���� ���ͷ��̶��
							tmpOper[len++] = 48 + atoi(lexeme);
						}
						lexeme = strtok_s(NULL, delmt, &context);
					}
					infix_to_postfix(postfix, tmpOper);
					i = 0;
					while (i < listCnt) {	// �Ҵ繮�� ���� �����̸��� ������ �����ߴٸ� Ÿ�԰� �� ���� �����϶�.
						if (strcmp(tmpVar, symboltable[i].var) == 0) {
							symboltable[i].value.intVar = eval(postfix);
							symboltable[i].type = INT;
							break;
						}
						i++;
					}
					if (i >= listCnt) {	// �����̸��� �ɺ����̺��� ã�� ���ߴٸ� ��, �űԺ������
						VARIABLE tmpStructVal;
						strcpy_s(tmpStructVal.var, SIZE, tmpVar);
						tmpStructVal.type = INT;
						tmpStructVal.value.intVar = eval(postfix);	// �ӽ� ����ü�� ����
						id(R, symboltable, tmpStructVal);	// �ĺ���(����) �����϶�� �Ѱ��ش�.
						//printf("listCnt: %d\n", listCnt);
					}
				}
				break;	// �ϴ� ���� ã������ ���̻� �������ʿ����
			}

			i++;
		}

		if (i >= listCnt) {	// �ش� ������ �ɺ����̺� �����ٸ�
			return -1;	// ���ǵ��� ���� ���� �̿��� ����
		}
	}
	else {	// ���� ��ū�� ������ ���
		// ���������� ��길 �����ϵ���
		tmpOper[len++] = 48 + atoi(lexeme);	// �ϴ� �� ���� �迭���ÿ� �־����
		while (lexeme != NULL) {
			if (strcmp(lexeme, "+") == 0) {
				tmpOper[len++] = '+';
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if (strcmp(lexeme, "-") == 0) {
				tmpOper[len++] = '-';
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if (strcmp(lexeme, "*") == 0) {
				tmpOper[len++] = '*';
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if (strcmp(lexeme, "/") == 0) {
				tmpOper[len++] = '/';
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if (atoi(lexeme) == 0) {	// �������
				i = 0;
				while (i < listCnt) {
					if (strcmp(lexeme, symboltable[i].var) == 0) {
						if (symboltable[i].type == INT) {	// ������ �������
							tmpOper[len++] = 48 + symboltable[i].value.intVar;	// �ش� ������ �������� �迭���ÿ� ����ش�.
						}
						else {	// �� ���� Ÿ����
							return -1;	// error-�߸��� ����
						}
					}
					i++;
				}

				if (i >= listCnt) {	// �ش� ������ �ɺ����̺� �����ٸ�
					return -1;	// ���ǵ��� ���� ���� �̿��� ����
				}
			}
			else {	// ���� ���ͷ��̶��
				tmpOper[len++] = 48 + atoi(lexeme);
			}
			lexeme = strtok_s(NULL, delmt, &context);
		}
		infix_to_postfix(postfix, tmpOper);
		i = 0;
		while (i < listCnt) {	// �Ҵ繮�� ���� �����̸��� ������ �����ߴٸ� Ÿ�԰� �� ���� �����϶�.
			if (strcmp(tmpVar, symboltable[i].var) == 0) {
				symboltable[i].value.intVar = eval(postfix);
				symboltable[i].type = INT;
				break;
			}
			i++;
		}
		if (i >= listCnt) {	// �����̸��� �ɺ����̺��� ã�� ���ߴٸ� ��, �űԺ������
			VARIABLE tmpStructVal;
			strcpy_s(tmpStructVal.var, SIZE, tmpVar);
			tmpStructVal.type = INT;
			tmpStructVal.value.intVar = eval(postfix);	// �ӽ� ����ü�� ����
			id(R, symboltable, tmpStructVal);	// �ĺ���(����) �����϶�� �Ѱ��ش�.
			//printf("listCnt: %d\n", listCnt);
		}
	}

	return 1;
}

/* ���������� ���� ���� ���� ���꿡 ���� �Լ��� */
void initStack(StackType *s) {
	s->top = -1;
}

int is_empty(StackType *s) {
	return s->top == -1;
}

int is_full(StackType *s) {
	return s->top == MAX_SIZE_STACK - 1;
}

void push(StackType *s, int item) {
	if (is_full(s))
		exit(1);
	s->stack[++(s->top)] = item;
}

int pop(StackType *s) {
	if (is_empty(s))
		exit(1);
	return s->stack[(s->top)--];
}

int peek(StackType *s) {
	if (is_empty(s))
		exit(1);
	return s->stack[s->top];
}

int prec(char op) {
	switch (op) {
	case '+': case '-': return 1;
	case '*': case '/': return 2;
	}
	return -1;
}

void infix_to_postfix(char result[], const char exp[]) {
	int i = 0, resCnt = 0;
	char ch = 0, top_op = 0;
	int len = (int)strlen(exp);

	StackType s;
	initStack(&s);

	for (i = 0; i < len; i++) {
		ch = exp[i];

		switch (ch) {
		case '+': case '-': case '*': case '/':
			while (!is_empty(&s) && prec(peek(&s)) >= prec(ch)) {
				top_op = pop(&s);
				result[resCnt++] = top_op;
			}
			push(&s, ch);
			break;
		default:
			result[resCnt++] = ch;
			break;
		}
	}

	while (!is_empty(&s)) {
		result[resCnt++] = pop(&s);
	}
}

/* ���� �� �Լ� ����� ��ȯ*/
int eval(char exp[]) {
	int op1, op2, value, i = 0;
	int len = strlen(exp);
	char ch;
	StackType s;
	initStack(&s);
	for (i = 0; i < len; i++) {
		ch = exp[i];
		if (ch != '+' && ch != '-' && ch != '*' && ch != '/') {
			value = ch - '0';
			push(&s, value);
		}
		else {
			op2 = pop(&s);
			op1 = pop(&s);
			switch (ch) {
			case '+': push(&s, op1 + op2); break;
			case '-': push(&s, op1 - op2); break;
			case '*': push(&s, op1*op2); break;
			case '/': push(&s, op1 / op2); break;
			}
		}
	}
	return pop(&s);
}

/* Ư������ ���� �Լ��� �� ���α׷������� ���ڿ��� �����Ҷ� '\"'���� ����Ǵ°��� �������� ���ȴ� */
void eliminate(char *str, char ch)
{
	char temp[SIZE];
	int i, idx = 0;

	for (i = 0; str[i] != 0; i++) {
		if (str[i] != ch) {
			temp[idx] = str[i];
			idx++;
		}
	}
	temp[idx] = 0;
	idx = 0;
	for (i = 0; temp[i] != 0; i++) {
		str[idx] = temp[i];
		idx++;
	}

	str[idx] = 0;
}