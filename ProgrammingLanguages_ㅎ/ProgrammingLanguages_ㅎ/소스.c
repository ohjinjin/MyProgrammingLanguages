#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME "test.txt"
#define MAX_SIZE_STACK 100
#define SIZE    512
#define MEMSIZE 100
#define REGSIZE 10
#define 시작    00
#define 만약    02
#define 아니면  03
#define 반복    04
#define 출력    05
#define 끝      06
#define 연산    07
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
	char var[SIZE];	// 변수 이름
	TYPE type;	// 타입이 뭔지 저장할 필드

	union {
		int intVar;	// 정수형일 경우 저장될 공간
		char charVar[SIZE];	// 문자열일 경우 저장될 공간
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

/* Main 함수 */
int main(void) {
	char M[MEMSIZE][SIZE] = { 0, }, R[REGSIZE][SIZE] = { {0, }, };	// 메모리와 레지스터 용도의 이차원배열(문자열배열)을 생성
	VARIABLE symboltable[SIZE];	// 심볼테이블 생성
	int opLine = 0;
	memset(symboltable, 0, sizeof(symboltable));
	opLine = init(M, R, symboltable);	// 프로그래밍언어 ㅎ 문법에 맞춰 작성된 프로그램 소스코드의 내용을 모두 읽어들여 M에 저장하는 함수 호출

	while (1) {
		interpreter(M, R, symboltable, opLine);	// 명령어들을 한 행씩 인출해 소스코드를 해석및 실행 하는 인터프리터 함수 호출
	}

	return 0;
}

/* 파일을 열어 프로그래밍언어ㅎ 문법에 의해 기술된 소스코드 전체를 M이라는 문자열 배열에 몽땅 저장하는 함수(HDD to MM) */
int init(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[]) {
	int i = 0;
	char file[SIZE] = FILENAME;
	FILE *fSrc = NULL;
	errno_t err;

	if ((err = fopen_s(&fSrc, file, "r")) != 0) {
		fprintf(stderr, "%s\n", "파일 열기 실패");
		exit(1);
	}

	while (!feof(fSrc)) {
		fgets(M[i], SIZE, fSrc);
		i++;
	}

	fclose(fSrc);
	return i;
}

/* M문자열배열(메모리)에서 R문자열배열(레지스터)로 한문장씩 fetch하는 기능의 함수 (MM to Register) 어떤 명령어까지 읽었는지를 프로그램카운터가 기억하고 있다 */
void fetch(char M[][SIZE], char R[][SIZE], int *ProgramCounter) {
	strcpy_s(R[0], SIZE, M[*ProgramCounter]);
	(*ProgramCounter)++;
}

/* 인터프리터_한줄씩 메모리의 명령어들을 레지스터로 옮긴 후(fetch) lexer어휘분석기를 통해 명령어의 opcode(토큰)를 해독하고 논터미널에 맞는 procedure를 실행하도록 반환받아서 문장을 실행한다. */
void interpreter(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int opLine) {
	static int ProgramCounter = 0;
	int opcode = -1;

	fetch(M, R, &ProgramCounter);
	opcode = lexer(M, R);
	stmt(opcode, M, R, symboltable, &ProgramCounter, opLine);	// 어떤 논터미널이 들어온건지도 stmt에 함께 전달해준다
}

/* 레지스터에 담겨있는 첫번째 문자열을 oper배열로 복사해와 delmt를 기준으로 자른 뒤
* 양분된 문자열의 두번째 문자열은 context가 참조하고 첫번째 문자열은 반환받아 lexeme에 저장한다.
* 첫 번째 문자열 lexeme의 값이 무엇인지에 대한 opcode를 반환해준다. 논터미널을 구분하는 역할의 함수 */
int lexer(char M[][SIZE], char R[][SIZE]) {
	char delmt[] = " ,\t\n(){}.";
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };

	strcpy_s(oper, SIZE, R[0]);

	lexeme = strtok_s(oper, delmt, &context);

	if (lexeme != NULL) {
		if (strcmp(lexeme, "시작") == 0)   return 시작;
		if (strcmp(lexeme, "만약") == 0)   return 만약;
		if (strcmp(lexeme, "아니면") == 0) return 아니면;
		if (strcmp(lexeme, "반복") == 0)   return 반복;
		if (strcmp(lexeme, "출력") == 0)   return 출력;
		if (strcmp(lexeme, "끝") == 0)     return 끝;

		return 연산;
	}
	return -1;
}

/* <stmt> 논터미널에 대한 procedure 정의
* <stmt> ::= <expr> | <assign_stmt> | <if_stmt> | <loop_stmt> | <output>
* 중 누가 선택되었는지에 따라 그 논터미널에 대응되는 procedure를 호출하게 됨 */
void stmt(int opcode, char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	static int logicRes = FALSE;

	// 인출해온 opcode로 어떤 논터미널이 선택되었는지에 따라 그에 해당하는 procedure를 호출한다.
	switch (opcode) {
	case 시작:	// 시작 기호에 대한 처리
		start(R, *ProgramCounter, opLine);
		break;
	case 만약:	// <if_stmt>
		logicRes = if_stmt(M, R, symboltable, ProgramCounter, opLine);
		break;
	case 아니면:	// <else_stmt>
		else_stmt(M, R, ProgramCounter, opLine, logicRes);
		break;
	case 반복:	// <loop_stmt>
		loop_stmt(M, R, symboltable, ProgramCounter, opLine);
		break;
	case 출력:	// <output>
		output(R, symboltable, listCnt);
		break;
	case 끝:	// 끝 기호에 대한 처리
		_getch();
		exit(1);
		break;
	case 연산:	// <assign_stmt>
		assign_stmt(R, symboltable, listCnt);
		break;
	}
}

/* 시작기호에 해당 현재 어디까지 읽었는가를 기억하는 변수 ProgramCounter 문장을 다읽을때까지 끝기호를 만나지못하면 프로그램이 정상종료되지 못했으므로 error 를 throw한다 */
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

/* 논터미널 <id>에 대한 procedure로서
* 프로그래밍언어ㅎ가 동적타이핑을 지원하므로
* 변수에 대한 선언문이 없으니 할당문에서 호출될 함수이다.
* symboltable에 변수를 생성해주는 역할의 함수 */
void id(char R[][SIZE], VARIABLE symboltable[], VARIABLE tmpStructVal) {
	symboltable[listCnt] = tmpStructVal;	// 새로 만든 VARIABLE을 symboltable에 저장
	listCnt++;	// symboltable에 몇개의 변수가 저장되어있는지 기억하는 용도의 전역변수
}

/* 논터미널 <if_stmt>에 해당하는 procedure로서
* conditional_expr을 체크한 이후 if절 혹은 else절이 실행되게 된다. */
int if_stmt(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	char oper[SIZE] = { 0, };
	char *tmpLex = NULL;
	char *context = NULL;

	if (conditional_expr(M, R[0], symboltable, ProgramCounter, opLine)) {	// 조건식이 참인경우
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

/* 논터미널 <cond_expr>에 해당하는 procedure로서 조건식에 대한 TRUE of FALSE 값을 반환하는 함수 */
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

	lexeme = strtok_s(oper, delmt, &context);	// 괄호 전까지 떼고
	lexeme = strtok_s(NULL, delmt, &context);	// 변수명 담아두기

	// 만약 기존에 변수가 존재한다면 미리 원본 값을 복사해둔다
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

	// 비교연산자 저장해두기
	lexeme = strtok_s(NULL, delmt, &context);
	strcpy_s(logicOper, SIZE, lexeme);

	// 피연산자 확인
	lexeme = strtok_s(NULL, delmt, &context);
	if (atoi(lexeme) == 0) {	// 피연산자가 변수일 경우
		i = 0;
		// 만약 기존에 변수가 존재한다면 미리 타겟 값을 복사해둔다
		while (i < listCnt) {
			if (strcmp(lexeme, symboltable[i].var) == 0) {
				if ((symboltable[i].type == INT) && (firstOperandType == INT)) {	// 첫번째 피연산자와 타입이 정수로 같다면
					targetInt = symboltable[i].value.intVar;
				}
				else if ((symboltable[i].type == STR) && (firstOperandType == STR)) {	// 첫번째 피연산자와 타입이 문자열로 같다면
					strcpy_s(targetStr, SIZE, symboltable[i].value.charVar);
				}
				else {
					printf("정수와 문자열의 비교는 불가합니다.");
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
			printf("정수와 문자열의 비교는 불가합니다.");
			return -1;	//이렇게해도되나
		}
	}
	else {	// 피연산자가 숫자라면
		if (firstOperandType == INT) {
			targetInt = atoi(lexeme);
		}
		else {
			printf("정수와 문자열의 비교는 불가합니다.");
			return -1;	//이렇게해도되나
		}
	}

	// 정수형끼리 비교의 경우
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
	// 문자열형끼리 비교의 경우
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

/* 논터미널로 존재하지는 않지만 판독성을 위하여 함수를 분리하였다.
* conditional_expr을 체크한 이후 if절 false반환시 else절이 실행되게 된다. */
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

/* 논터미널 <loop_stmt>에 해당하는 procedure로서
* conditional_expr의 반환값이 TRUE인 경우 블록 내 코드들이 반복적으로 실행되게 된다. */
void loop_stmt(char M[][SIZE], char R[][SIZE], VARIABLE symboltable[], int *ProgramCounter, int opLine) {
	char *lexeme = NULL;
	char *context = NULL;
	char oper[SIZE] = { 0, };
	int i = 0, limit = 0;
	int tmpProgramCounter = *ProgramCounter;
	int opcode = -1;

	strcpy_s(oper, SIZE, R[0]);

	char condStmt[SIZE] = { 0, };
	strcpy_s(condStmt, SIZE, R[0]);	// 반복조건문 담아놓기
	lexeme = strtok_s(oper, delmt, &context);	// 괄호 떼고
	lexeme = strtok_s(NULL, delmt, &context);	// 조건식부분만 가져오기

	while (!(conditional_expr(M, condStmt, symboltable, ProgramCounter, opLine))) {	// 매번 조건문을 확인하도록
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

/* 논터미널 <output>에 해당하는 procedure로 type에 따라 밸류를 알맞게 formating하여 출력한다 */
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
				printf("(정수형 변수) %s: %d\n", symboltable[i].var, symboltable[i].value.intVar);
			}
			else if (symboltable[i].type == STR) {
				printf("(문자열형 변수) %s: %s\n", symboltable[i].var, symboltable[i].value.charVar);
			}

			break;
		}
		i++;
	}

	if (i >= listCnt) {
		printf("정의되지 않은 변수 사용 에러\n");
	}
}

/* 논터미널 <assign_stmt>에 해당하는 procedure로, id에 expr의 결과를 할당한다. <expr>부분에서는 식이 타입에 맞게 평가된다 */
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
	strcpy_s(oper, SIZE, R[0]);	// R배열에 저장되어있는 한줄의 문장이 oper로 복사되고

	lexeme = strtok_s(oper, delmt, &context);	// 한 문장 전체를 delmt를 기준으로 양분하여 첫번째 문자열을 lexeme에, 두번째 문자열을 context에 저장해준다.
	strcpy_s(tmpVar, SIZE, lexeme);	// lexeme을 tmpVar에 복사해놓고

	lexeme = strtok_s(NULL, delmt, &context);	// 다음 토큰이
	if (strcmp(lexeme, "=") == 0) {	// "="연산자인지 확인하여
		if (!(expr(R, symboltable, listCnt, tmpVar, context))) {	// 성공적으로 수식 계산이 완료되지 않앗다면
			printf("잘못된 수식으로 인한 할당문 오류\n");
		}
	}
	else {
		printf("error 올바른 수식이 아닙니다.\n");
	}
}

/* 논터미널 <expr>에 해당하는 procedure로서 식(연산자,피연산자,타입 등)을 평가하고 수행결과에 대해 성공여부를 반환한다 */
int expr(char R[][SIZE], VARIABLE symboltable[], int listCnt, char tmpVar[], char expression[]) {
	char *lexeme = NULL;
	char *context = NULL;
	int i = 0, tmpValue = 0, len = 0;
	char tmpOper[SIZE] = { 0, };
	char postfix[SIZE] = { 0, };
	char *tmpLex = NULL;
	int isStr = FALSE;

	//철수\0= 철수 + 1 * 3, 철수 + 1 * 3이 expression에 담겨있음
	lexeme = strtok_s(expression, delmt, &context);	// 다음 토큰을 lexeme에 담고

	if (lexeme[0] == '\"') {	// 다음 토큰의 첫 글자가 더블코워트로 시작하는 경우
		/*
		while () {	// 한문장 다 끝날때 까지 반복
			// 피연산자가 정수면 error
			// 연산자는 +만 가능
			// 피연산자가 ""인경우 strcat()
			// 피연산자가 변수인경우 변수의 타입이 str가 맞으면 변수의 값을 strcat()
		}
		*/
		char tmpStr[SIZE] = { 0, };
		int flag = TRUE;	// 문자열 접합을 하는 것에 있어서 +연산자가 등장했었다는 표시를 하기위한 플래그 변수

		//strcpy_s(tmpStr, SIZE, lexeme);	// 일단 그 문자열리터럴을 임시 문자배열에 저장해놓는다
		while (lexeme != NULL) {	// 해당 문장이 끝날때까지
			if (strcmp(lexeme, "+") == 0) {
				flag = TRUE;
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if ((lexeme[0] == '\"') && (flag)) {	// 새로운 문자열 리터럴이 등장했고, 앞서서 +연산자도 있었다면
				strcat_s(tmpStr, SIZE, lexeme);	// tmpStr 뒤에 lexeme을 이어붙인다.
				flag = FALSE;	// 이항연산자인 +의 피연산자를 다썼다는것을 의미함
				lexeme = strtok_s(NULL, delmt, &context);
				continue;
			}
			if ((atoi(lexeme) == 0) && (flag)) {	// 변수가 등장했고, 앞서서 +연산자도 있었다면
				i = 0;
				while (i < listCnt) {	// 해당 변수가 존재하는 변수인지 확인
					if (strcmp(lexeme, symboltable[i].var) == 0) {	// 같은 이름의 변수가 심볼테이블에 있다면
						if (symboltable[i].type == STR) {	// 문자열 변수라면
							strcat_s(tmpStr, SIZE, symboltable[i].value.charVar);	// 해당 변수의 기존 값을 tmpStr 뒤에 이어붙인다.
						}
						else {
							return -1;	// error 타입이 맞지 않음 잘못된 수식
						}
						break;
					}
					i++;
				}

				if (i >= listCnt) {	// 해당 변수가 심볼테이블에 없었다면
					return -1;	// 정의되지 않은 변수 이용한 오류
				}
				flag = FALSE;	// 이항연산자인 +의 피연산자를 다썼다는것을 의미함
			}
			else {	// 그 외 연산자를 사용했다거나, 정수값을 더하려고했다거나 하는 경우엔
				return -1;	// 에러 발생-잘못된 수식 피연산자간의 타입오류
			}
			lexeme = strtok_s(NULL, delmt, &context);
		}

		i = 0;
		while (i < listCnt) {	// 할당문의 좌측 변수이름이 기존에 존재했다면 중복 저장할 것이 아니라 그 변수의 타입과 그 값을 갱신하라.
			if (strcmp(tmpVar, symboltable[i].var) == 0) {
				eliminate(tmpStr, '\"');
				strcpy_s(symboltable[i].value.charVar, SIZE, tmpStr);	// 해당 변수의 값에 tmpStr을 복사해준다.
				symboltable[i].type = STR;
				break;
			}
			i++;
		}
		if (i >= listCnt) {	// 변수이름을 심볼테이블에서 찾지 못했다면 즉, 신규변수라면
			VARIABLE tmpStructVal;
			strcpy_s(tmpStructVal.var, SIZE, tmpVar);	// 변수이름 지어준대로 복사하고
			tmpStructVal.type = STR;	// 변수 타입을 STR로 지정하고
			eliminate(tmpStr, '\"');
			strcpy_s(tmpStructVal.value.charVar, SIZE, tmpStr);	// 해당 변수의 값에 tmpStr을 복사해주고
			id(R, symboltable, tmpStructVal);	// 식별자(변수) 생성하라고 넘겨준다.
			//printf("listCnt: %d\n", listCnt);
		}
	}
	else if (atoi(lexeme) == 0) {	// 다음 토큰이 변수명인 경우
		/*그 변수의 타입을 분석한 다음 위(문자열 관련 연산) 아래(정수 관련 연산) 복붙해주면 됨 */
		i = 0;
		while (i < listCnt) {	// 해당 변수가 존재하는 변수인지 확인
			if (strcmp(lexeme, symboltable[i].var) == 0) {	// 같은 이름의 변수가 심볼테이블에 있다면
				if (symboltable[i].type == STR) {	// 문자열 변수라면
					char tmpStr[SIZE] = { 0, };
					int flag = FALSE;	// 문자열 접합을 하는 것에 있어서 +연산자가 등장했었다는 표시를 하기위한 플래그 변수

					strcpy_s(tmpStr, SIZE, symboltable[i].value.charVar);	// 해당 변수의 기존 값을 임시 문자배열에 저장해놓는다
					lexeme = strtok_s(NULL, delmt, &context);

					while (lexeme != NULL) {	// 해당 문장이 끝날때까지
						if (strcmp(lexeme, "+") == 0) {
							flag = TRUE;
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if ((lexeme[0] == '\"') && (flag)) {	// 새로운 문자열 리터럴이 등장했고, 앞서서 +연산자도 있었다면
							strcat_s(tmpStr, SIZE, lexeme);	// tmpStr 뒤에 lexeme을 이어붙인다.
							flag = FALSE;	// 이항연산자인 +의 피연산자를 다썼다는것을 의미함
							lexeme = strtok_s(NULL, delmt, &context);
							continue;
						}
						if ((atoi(lexeme) == 0) && (flag)) {	// 변수가 등장했고, 앞서서 +연산자도 있었다면
							i = 0;
							while (i < listCnt) {	// 해당 변수가 존재하는 변수인지 확인
								if (strcmp(lexeme, symboltable[i].var) == 0) {	// 같은 이름의 변수가 심볼테이블에 있다면
									if (symboltable[i].type == STR) {	// 문자열 변수라면
										strcat_s(tmpStr, SIZE, symboltable[i].value.charVar);	// 해당 변수의 기존 값을 tmpStr 뒤에 이어붙인다.
									}
									else {
										return -1;	// error 타입이 맞지 않음 잘못된 수식
									}
									break;
								}
								i++;
							}

							if (i >= listCnt) {	// 해당 변수가 심볼테이블에 없었다면
								return -1;	// 정의되지 않은 변수 이용한 오류
							}
							flag = FALSE;	// 이항연산자인 +의 피연산자를 다썼다는것을 의미함
						}
						else {	// 그 외 연산자를 사용했다거나, 정수값을 더하려고했다거나 하는 경우엔
							return -1;	// 에러 발생-잘못된 수식 피연산자간의 타입오류
						}
						lexeme = strtok_s(NULL, delmt, &context);
					}
					i = 0;
					while (i < listCnt) {	// 할당문의 좌측 변수이름이 기존에 존재했다면 타입과 그 값을 갱신하라. (원래 가지던 타입은 무관_동적타이핑이니까)
						if (strcmp(tmpVar, symboltable[i].var) == 0) {
							eliminate(tmpStr, '\"');
							strcpy_s(symboltable[i].value.charVar, SIZE, tmpStr);	// 해당 변수의 값에 tmpStr을 복사해준다.
							symboltable[i].type = STR;
							break;
						}
						i++;
					}
					if (i >= listCnt) {	// 변수이름을 심볼테이블에서 찾지 못했다면 즉, 신규변수라면
						VARIABLE tmpStructVal;
						strcpy_s(tmpStructVal.var, SIZE, tmpVar);	// 변수이름 지어준대로 복사하고
						tmpStructVal.type = STR;	// 변수 타입을 STR로 지정하고
						eliminate(tmpStr, '\"');
						strcpy_s(tmpStructVal.value.charVar, SIZE, tmpStr);	// 해당 변수의 값에 tmpStr을 복사해주고
						id(R, symboltable, tmpStructVal);	// 식별자(변수) 생성하라고 넘겨준다.
						//printf("listCnt: %d\n", listCnt);
					}
				}
				else if (symboltable[i].type == INT) {	//	정수형이라면
					tmpOper[len++] = 48 + symboltable[i].value.intVar;	// 일단 그 수를 배열스택에 넣어놓고
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
						if (atoi(lexeme) == 0) {	// 변수라면
							i = 0;
							while (i < listCnt) {
								if (strcmp(lexeme, symboltable[i].var) == 0) {
									if (symboltable[i].type == INT) {	// 정수형 변수라면
										tmpOper[len++] = 48 + symboltable[i].value.intVar;	// 해당 변수의 정수값을 배열스택에 담아준다.
									}
									else {	// 그외의 타입은
										return -1;	// error-잘못된 수식
									}
								}
								i++;
							}

							if (i >= listCnt) {	// 해당 변수가 심볼테이블에 없었다면
								return -1;	// 정의되지 않은 변수 이용한 오류
							}
						}
						else {	// 정수 리터럴이라면
							tmpOper[len++] = 48 + atoi(lexeme);
						}
						lexeme = strtok_s(NULL, delmt, &context);
					}
					infix_to_postfix(postfix, tmpOper);
					i = 0;
					while (i < listCnt) {	// 할당문의 좌측 변수이름이 기존에 존재했다면 타입과 그 값을 갱신하라.
						if (strcmp(tmpVar, symboltable[i].var) == 0) {
							symboltable[i].value.intVar = eval(postfix);
							symboltable[i].type = INT;
							break;
						}
						i++;
					}
					if (i >= listCnt) {	// 변수이름을 심볼테이블에서 찾지 못했다면 즉, 신규변수라면
						VARIABLE tmpStructVal;
						strcpy_s(tmpStructVal.var, SIZE, tmpVar);
						tmpStructVal.type = INT;
						tmpStructVal.value.intVar = eval(postfix);	// 임시 구조체를 만들어서
						id(R, symboltable, tmpStructVal);	// 식별자(변수) 생성하라고 넘겨준다.
						//printf("listCnt: %d\n", listCnt);
					}
				}
				break;	// 일단 변수 찾았으면 더이상 루프돌필요없음
			}

			i++;
		}

		if (i >= listCnt) {	// 해당 변수가 심볼테이블에 없었다면
			return -1;	// 정의되지 않은 변수 이용한 오류
		}
	}
	else {	// 다음 토큰이 정수인 경우
		// 정수끼리의 계산만 가능하도록
		tmpOper[len++] = 48 + atoi(lexeme);	// 일단 그 수를 배열스택에 넣어놓고
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
			if (atoi(lexeme) == 0) {	// 변수라면
				i = 0;
				while (i < listCnt) {
					if (strcmp(lexeme, symboltable[i].var) == 0) {
						if (symboltable[i].type == INT) {	// 정수형 변수라면
							tmpOper[len++] = 48 + symboltable[i].value.intVar;	// 해당 변수의 정수값을 배열스택에 담아준다.
						}
						else {	// 그 외의 타입은
							return -1;	// error-잘못된 수식
						}
					}
					i++;
				}

				if (i >= listCnt) {	// 해당 변수가 심볼테이블에 없었다면
					return -1;	// 정의되지 않은 변수 이용한 오류
				}
			}
			else {	// 정수 리터럴이라면
				tmpOper[len++] = 48 + atoi(lexeme);
			}
			lexeme = strtok_s(NULL, delmt, &context);
		}
		infix_to_postfix(postfix, tmpOper);
		i = 0;
		while (i < listCnt) {	// 할당문의 좌측 변수이름이 기존에 존재했다면 타입과 그 값을 갱신하라.
			if (strcmp(tmpVar, symboltable[i].var) == 0) {
				symboltable[i].value.intVar = eval(postfix);
				symboltable[i].type = INT;
				break;
			}
			i++;
		}
		if (i >= listCnt) {	// 변수이름을 심볼테이블에서 찾지 못했다면 즉, 신규변수라면
			VARIABLE tmpStructVal;
			strcpy_s(tmpStructVal.var, SIZE, tmpVar);
			tmpStructVal.type = INT;
			tmpStructVal.value.intVar = eval(postfix);	// 임시 구조체를 만들어서
			id(R, symboltable, tmpStructVal);	// 식별자(변수) 생성하라고 넘겨준다.
			//printf("listCnt: %d\n", listCnt);
		}
	}

	return 1;
}

/* 후위연산을 위한 스택 관련 연산에 대한 함수들 */
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

/* 수식 평가 함수 결과를 반환*/
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

/* 특정문자 제거 함수로 이 프로그램에서는 문자열을 저장할때 '\"'까지 저장되는것을 막기위해 사용된다 */
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