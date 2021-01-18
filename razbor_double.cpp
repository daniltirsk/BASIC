#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include "razbor_double.h"
#include "small_basic.h"


// math functions

void exec_sin(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = sin(temp);
}

void exec_cos(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = cos(temp);
}

void exec_tan(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = tan(temp);
}

void exec_log(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = log(temp);
}

void exec_exp(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = exp(temp);
}

void exec_abs(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = abs(temp);
}

void exec_atn(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = atan(temp);
}

void exec_sqr(double& result) {
	double temp;
	get_token();
	eval_exp6(temp);
	result = sqrt(temp);
}


//Константы, используемые для вызова serror() при синтаксических ошибках
enum error_msg{
	SYNTAX, UNBAL_PARENS, NO_EXP, NOT_VAR, LAB_TAB_FULL, DUP_LAB,
	UNDEF_LAB, THEN_EXP, TO_EXP, TOO_MNY_FOR, NEXT_WO_FOR, TOO_MNY_GOSUB,
	RET_WO_GOSUB, MISS_QUOTE
};

//Точка входа
void eval_exp(double &result){
	get_token();
	if(!*token){
		serror(NO_EXP); //нет выражения
		return;
	}
	eval_exp1(result);
	putback();//возвратить поседний считанный элемент во вводной поток
}

//Обработка реляционных операторов
void eval_exp1(double &result){
	//Реляционные операторы
	char relops[] = {GE,NE,LE,'<','>','=',0};
	double temp;
	register char op;

	eval_exp2(result);
	
	op = *token;
	if(strchr(relops,op)){
		get_token();
		eval_exp1(temp);
		switch(op){//выполнение реляционных операций
			case '<':
				result = result < temp;
				break;
			case LE:
				result = result <= temp;
				break;
			case '>':
				result = result > temp;
				break;
			case GE:
				result = result >= temp;
				break;
			case '=':
				result = result == temp;
			case NE:
				result = result != temp;
				break;
		}
	}
}

//Сложение и вычитание двух элементов
void eval_exp2(double &result){
	register char op;
	double temp;

	eval_exp3(result);

	while((op = *token) == '+' || op =='-'){

		get_token();
		eval_exp3(temp);

		switch(op){
			case '-':
				result = result - temp;
				break;
			case '+':
				result = result + temp;
				break;
		}
	}
}

//Умножение и деление 
void eval_exp3(double &result){
	register char op;
	double temp;

	eval_exp4(result);

	while((op = *token) == '*' || op =='/'){

		get_token();
		eval_exp4(temp);

		switch(op){
			case '*':
				result = result * temp;
				break;
			case '/':
				result = result / temp;
				break;
		}
	}
}

//Возведение в целую степень
void eval_exp4(double &result){
	double temp, ex;
	register int t;

	eval_exp5(result);

	if(*token == '^'){

		get_token();
		eval_exp4(temp);
		ex = result;

		if(temp == 0.0){
			result = 1.0;
			return; 
		}
		for(t = (int)temp-1; t>0; --t)
			result = result * ex;
	}
}

//Унарный + или -
void eval_exp5(double &result){
	register char op;
	op = 0;
	if((token_type == DELIMITER) && *token == '+' || *token == '-'){
		op = *token;
		get_token();
	}

	eval_exp6(result);

	if(op == '-')
		result = -result;
}

//Обработка выражения в скобках
void eval_exp6(double &result){

	if(*token == '(' ){

		get_token();
		eval_exp2(result);

		if(*token != ')')
			serror(UNBAL_PARENS);
		get_token();
	}
	else
		atom(result);
}

//Получение значения числа или переменной
void atom(double &result){
	switch(token_type){
		case VARIABLE:
			result = find_var(token);
			get_token();
			return;
		case NUMBER:
			result = atof(token);
//			printf("result=%lf\n",result);
			get_token();
			return;
		case COMMAND:
			switch (tok) {
				case SIN:
					exec_sin(result);
					break;
				case COS:
					exec_cos(result);
					break;
				case TAN:
					exec_tan(result);
					break;
				case LOG:
					exec_log(result);
					break;
				case EXP:
					exec_exp(result);
					break;
				case ABS:
					exec_abs(result);
					break;
				case ATN:
					exec_atn(result);
					break;
				case SQR:
					exec_sqr(result);
					break;
			}
			return;
		default:
			serror(SYNTAX);
	}
}

//Найти значение переменной
double find_var(char *s){
	if(!isalpha(*s)){
		serror(NOT_VAR); //не переменная
		return 0.0;
	}
	return variables[toupper(*token) - 'A'];
}

//Отображение синтаксических ошибок
void serror(int error){
	char *p,*temp;
	int linecount = 0;
	register int i;
	const char *e[] = {
		"Sintacsis error!",//"Синтаксическая ошибка",
		"Nezakrytye skobri!",//Незакрытые скобки",
		"Not express for razbora!",//Нет выражения для разбора",
		"Need sign =!",//Требуется знак равенства",
		"Not variable!",//Не переменная",
		"Perepolnennaya tablica metok!",//Переполненная таблица меток",
		"Dublir metka!",//Дублирующаяся метка",
		"Neopredelennaya metka!",//Неопределенная метка",
		"Need THEN",//Требуется THEN",
		"NeedТ TO",//ребуется TO",
		"Very many vlohennyh ciclov FOR",//Слишком много вложенных циклов FOR",
		"NEXT need FOR",//NEXT требует FOR",
		"Very many dlogeniy GOSUB",//Слишком много вложений GOSUB",
		"RETURN without GOSUB",//ETURN без GOSUB",
		"Need double kovychki!"//Требуются двойные кавычки"
	};
	std::cout<<e[error];

	p = p_buf;
	while(p != prog){ //найти номер строки, содержащей ошибку
		p++;
		if(*p == '\r'){
			linecount++;
		}
	}
//	std::cout<<"в строке "<<linecount<<".\n";
	std::cout<<" in string "<<linecount<<".\n";
	temp = p; //отобразить строку с ошибкой
	for(i=0; i<20 && p>p_buf && *p !='\n'; i++, p--);
	for(; p<=temp; p++)
		std::cout<<*p;
	throw(1);
}

//Получение следующего элемента
int get_token(){
	register char *temp;
	token_type = 0;
	tok = 0;
	temp = token;
	if(*prog == '\0'){//Конец строки
		*token = 0;
		tok = FINISHED;
		return (token_type = DELIMITER);
	}
	while(is_sp_tab(*prog))
		++prog;//пропускаем пробелы

	if(*prog == '\r'){//crlf
		++prog; ++prog;
		tok = EOL;
		*token = '\r';
		token[1] = '\n';
		token[2] = 0;
		return (token_type = DELIMITER);
	}

	if(strchr("<>",*prog)){//проверка двойных операторов
		switch(*prog){
			case '<':
				if(*(prog+1) == '>'){
					prog++; prog++;
					*temp = NE;
				}
				else if(*(prog+1) == '='){
					prog++; prog++;
					*temp = LE;
				}
				else{
					prog++;
					*temp = '<';
				}
				temp++;
				*temp = '\0';
				break;
			case '>':
				if(*(prog+1) == '='){
					prog++; prog++;
					*temp = GE;
				}
				else{
					prog++;
					*temp = '>';
				}
				temp++;
				*temp = '\0';
				break;
		}
		return (token_type = DELIMITER);
	}
	if(strchr("+-*^/=;(),",*prog)){ //разделитель
		*temp = *prog;
		prog++;
		temp++;
		*temp = 0;
		return (token_type = DELIMITER);
	}

	if(*prog == '"'){
		prog++;
		while(*prog != '"' && *prog != '\r')
			*temp++ = *prog++;
		if(*prog == '\r')
			serror(MISS_QUOTE);
		prog++;
		*temp =0;
		return (token_type = QUOTE);
	}

	if(isdigit(*prog)){
		while(!isdelim(*prog))
			*temp++ = *prog++;
		*temp = '\0';
//		printf("temp=%s\n",temp);
		return (token_type = NUMBER);
	}

	if(isalpha(*prog)){
		while(!isdelim(*prog))//Читаем до тех пор пока не встретим разделитель " ; ,+=<>/*%^()"
			*temp++ = *prog++;
		token_type = STRING;
	}
	*temp = '\0';

	//Не является ли строка командой или переменной
	if(token_type == STRING){
		
		tok = look_up(token); //преобразование во внутреннее представление
		if(!tok) token_type = VARIABLE;
		else token_type = COMMAND;
	}
	return token_type;
}

//Возвратить элемент во вводной поток
void putback(){
	char *t;
	t = token;
	for(; *t; t++)
		prog--;
}

//Поиск внутреннего представления в таблице
int look_up(char *s){
	register int i;
	char *p;
//преобразование в нижнийрегистр
	p = s;
	while(*p){
		*p = tolower(*p);
		p++;
	}
	
	for(i = 0; *table[i].command; i++){
//		printf("%d %s %s strcmp=%d\n",i,table[i].command, s,strcmp(table[i].command, s)); getch();
		if(!strcmp(table[i].command, s)){
//			printf("OK! ");
			return table[i].tok;
		}
	}
	return 0; //неизвестная команда
}

//Возвратить TRUЕ, если с представляет собой разделитель
int isdelim(char c){
	if(strchr(" ; ,+=<>/*%^()", c) || c ==9 || c =='\r' || c ==0)
		return 1;
	return 0;
}

//Возвратить 1, если с - пробел или табулятор
int is_sp_tab(char c){
	if(c == ' ' || c == '\t')
		return 1;
	else
		return 0;
}