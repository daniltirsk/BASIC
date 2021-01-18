#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
//#include "razbor.h"
#include "razbor_double.h"
#include "small_basic.h"

char *prog;//Указатель на точку входа в программу
char *p_buf;//Указатель на точку старта программы

//int variables[26]={0,0}; //Доступно 26 пересменных A - Z
double variables[26]={0.0}; //Доступно 26 пересменных A - Z

commands table[] = {
	"print", PRINT,
	"input", INPUT,
	"if", IF,
	"then", THEN,
	"goto", GOTO,
	"for", FOR,
	"next", NEXT,
	"to", TO,
	"gosub", GOSUB,
	"return", RETURN,
	"end", END,
	"sin",SIN,
	"cos",COS,
	"tan",TAN,
	"exp",EXP,
	"log",LOG,
	"abs",ABS,
	"sqr",SQR,
	"atn",ATN,
	"", END
};

char token[80];
char token_type, tok;

//label lookup table
struct label{
	char name[LAB_LEN];//label
	char *p;//points to label's location in source file
} label_table[NUM_LAB];

//support for FOR loops
struct for_stack{
	int var;//counter variable
//	int target;//target value
	double target;//target value
	char *loc;//place in source code to loop to
} fstack[FOR_NEST]; //stack for FOR/NEXT loop

char *gstack[SUB_NEST];//stack for gosub

int ftos;//index to top of FOR stack
int gtos;//index to top GOSUB stack

void print();
void scan_labels();
void find_eol();//Find the start of the next line
void exec_goto();
void exec_if();
void exec_for();
void next();
void fpush(struct for_stack i);
void input();
void gosub();
void greturn();
void gpush(char *s);
void label_init();
void assignment();
char *find_label(char*s);
char *gpop();
struct for_stack fpop();
int load_program(char *p, char *fname);
int get_next_label(char *s);

enum error_msg{
	SYNTAX, UNBAL_PARENS, NO_EXP, EQUAL_EXP,
	NOT_VAR, LAB_TAB_FULL, DUP_LAB, UNDEF_LAB,
	THEN_EXP, TO_EXP, TOO_MNY_FOR, NEXT_WO_FOR,
	TOO_MNY_GOSUB, RET_WO_GOSUB, MISS_QUOTE
};

int main(int argc, char *argv[]){
//	if(argc!=2){
//		std::cout<<"Usage: sbasic <filename>\n";
//		exit(1);
//	}
	argv[1] = new char[500];
	strcpy(argv[1],"test3.bas");

	//allocate memory for the program
	prog = new char [PROG_SIZE];
	if(!prog){
		std::cout<<"Allocatin Failure";
		exit(1);
	}
	p_buf = prog;

	//load the program to execute
	if(!load_program(prog, argv[1])) exit(1);
	//begin try block   
	try{
		scan_labels();//find the labels in the program
		ftos = 0;//initialize the FOR stack index
		gtos = 0;//initialize the GOSUB stack index
		do{
			token_type = get_token();
			//std::cout << "tok: " << token << std::endl;
			//check for assignment statement
			if(token_type == VARIABLE){

				putback();//Возвратить элемент во вводной поток
				assignment();//must be assignment statement
//				printf("token_type=%d token=%lf\n",token_type,variables[var]);
			}
			else //is command
				switch(tok){
					/*case SIN:
						exec_sin();
						break;*/
					case PRINT:
						print();
						break;
					case GOTO:
						exec_goto();
						break;
					case IF:
						exec_if();
						break;
					case FOR:
						exec_for();
						break;
					case NEXT:
						next();
						break;
					case INPUT:
						input();
						break;
					case GOSUB:
						gosub();
						break;
					case RETURN:
						greturn();
						break;
					case END:
						return 0;
				}
		} while(tok != FINISHED);
	}//end of try block

	catch(int){
		return 0;//fatal error
	}
	return 0;
}

//load program
int load_program(char *p, char *fname){
	std::ifstream in(fname, std::ios::in | std::ios::binary);
	int i=0;

	if(!in){
		std::cout<<"File not found ";
		std::cout<<"- besure to specify /BAS extension. \n";
		return 0;
	}

	i=0;
	do{
		*p = in.get();
		p++;
		i++;
	}while(!in.eof() && i<PROG_SIZE);

	//null terminate the program
	if(*(p-2)==0x1a)  
		*(p-2) = '\0'; //discard eof marker
	else
		*(p-1) = '\0';

	in.close();
	return 1;
}

//Find all labels
void scan_labels(){
	int addr;
	char *temp;
	label_init(); //zero all labels
	temp = prog; //save pointer to top of program

	//if the first token in the file is a label
	get_token();
	if(token_type == NUMBER){
		strcpy(label_table[0].name, token);
		label_table[0].p = prog;
	}

	find_eol();
	do{
		get_token();
		if(token_type == NUMBER){
			addr = get_next_label(token);
			if(addr == -1 || addr == -2){
				(addr == -1) ? serror(LAB_TAB_FULL):serror(DUP_LAB);
			}
			strcpy(label_table[addr].name, token);
		//save current location in program
			label_table[addr].p = prog;
		}
	//if not on a blank line, find next line
		if(tok!=EOL)
			find_eol();
	} while(tok!=FINISHED);
	prog = temp; //restore original location
}

//Find the start of the next line
void find_eol(){
	while (*prog!='\n' && *prog!='\0')
		++prog;
	if(*prog)
		prog++;
}

//Return index of next free position in label array
// -1 is returned if the array is full/
// -2 is returned when duplicate label is found.
int get_next_label(char *s){
	register int i;
	for(i=0; i<NUM_LAB; ++i){
		if(label_table[i].name[0] == 0)
			return i;
		if(!strcmp(label_table[i].name, s))
			return -2;
	}
	return -1;
}

//Find location of given label. A null is returned if label is not foubnd
//otherwise a pointer to the position of the label is returned
char *find_label(char *s){
	register int i;
	for(i=0; i<NUM_LAB; ++i)
		if(!strcmp(label_table[i].name, s))
			return label_table[i].p;
	char returnValue = '\0';
	return &returnValue;//error condition
}

//Initialize the array that holds the labels.
//By convention, a null label name indicates that the array position is unused/
void label_init(){
	register int i;
	for(i=0; i<NUM_LAB; ++i)
		label_table[i].name[0] = '\0';
}

//Assign a variable a value
void assignment(){
	int var;//, value;
	double value;
	//get the variable name
	get_token();
	if(!isalpha(*token)){
		serror(NOT_VAR);
		return;
	}
	//convert to index into variable table
	var = toupper(*token) - 'A';
	printf("%s = ",token);
	//get the equal sign
	get_token();
	if(*token != '='){
		serror(EQUAL_EXP);
		return;
	}
	//get the value to assign
	eval_exp(value);
	printf("%lf\n",value);
	//assign the value
	variables[var] = value;
}

//Execute a simple version of the BASIC PRINT statement
void print(){
//	int result;
	double result;
	int len=0, spaces;
	char last_delim, str[80];
	do{
		get_token(); //get next list item
		if(tok == EOL || tok == FINISHED)
			break;
		if(token_type == QUOTE){//is string
			std::cout<<token;
			len += strlen(token);
			get_token();
		}
		else{//is expression
			putback();
			eval_exp(result);//Точка входа в разбор выражения
			get_token();
//			printf("X=%lf\n",result);
			std::cout<<result;
//			itoa(result, str, 10);
			sprintf(str, "%lf", result);
			len += strlen(str); //save length
		}
		last_delim = *token;

		//if comma, move to next tab stop
		if(*token == ','){
		//compute number of spaces to move to next tab
			spaces = 8 - (len%8);
			len +=spaces; //add in the tabbing position
			while(spaces){
				std::cout<<" ";
				spaces--;
			}
		}
		else if(*token == ';'){
			std::cout<<" ";
			len++;
		}
		else if(tok != EOL && tok != FINISHED)
			serror(SYNTAX);
	} while(*token == ';' || *token == ',');
	
	if(tok == EOL || tok == FINISHED){
		if(last_delim != ';' && last_delim != ',')
			std::cout << std::endl;
	}
	else
		serror(SYNTAX);
}

//Execute a GOTO statement
void exec_goto(){
	char *loc;
	get_token(); //get label to go to
	//find the location of the label
	loc = find_label(token);
	if(loc == NULL)
		serror(UNDEF_LAB); //label not defined
	else
		prog = loc; //start program running at that loc
}

//Execute an IF statement
void exec_if(){
//	int result;
	double result;
	char op;

	eval_exp(result);//get value of expression
	if(result){//is true so process target of IF
		get_token();
		if(tok != THEN){
			serror(THEN_EXP);
			return;
		}//else, target statement will be executed
	}
	else
		find_eol();//find start of next line
}

//Execute a FOR loop
void exec_for(){
	struct for_stack stckvar;
//	int value;
	double value;
	get_token();// read the control variable
	if(!isalpha(*token)){
		serror(NOT_VAR);
		return;
	}
	//save index of control var
	stckvar.var = toupper(*token) - 'A';
	get_token(); //read the equal sign
	if(*token != '='){
		serror(EQUAL_EXP);
		return;
	}
	eval_exp(value);// get initial value

	variables[stckvar.var] = value;

	get_token();
	if(tok != TO)
		serror(TO_EXP);// read and discard the TO

	eval_exp(stckvar.target); //get least value

	//if loop can execute at least once, push info on stack
	if(value >= variables[stckvar.var]){
		stckvar.loc = prog;
		fpush(stckvar);
	}
	else //other, skip loop code altogether
		while(tok != NEXT)
			get_token();
}

//Execute a NEXT statement
void next(){
	struct for_stack stckvar;
	stckvar = fpop(); //read the loop info
	variables[stckvar.var]++; //increment control var
	//if done, return
	if(variables[stckvar.var] > stckvar.target)
		return;
	fpush(stckvar); //otherwise, restore the info
	prog = stckvar.loc; //loop
}

//Push the FOR stack
void fpush(struct for_stack stckvar){
	if(ftos == FOR_NEST)
		serror(TOO_MNY_FOR);
	fstack[ftos] = stckvar;
	ftos++;
}

//POP the FOR stack
struct for_stack fpop(){
	if(ftos == 0)
		serror(NEXT_WO_FOR);
	ftos--;
	return(fstack[ftos]);
}

//Execute a simple form of the BASIC INPUT command
void input(){
	char var;
	int i;
	get_token(); //see if prompt string is present
	if(token_type == QUOTE){
		std::cout<<token;// if so, print it and check for comma
		get_token();
		if(*token != ',')
			serror(SYNTAX);
		get_token();
	}
	else
		std::cout<<"? "; //otherwise, prompt with?
	var = toupper(*token) - 'A'; //get the input var

	std::cin>>i; //read input
	variables[var] = i; // store it
}

//Execute a GOSUB command
void gosub(){
	char *loc;
	get_token();
	//find the label to call
	loc = find_label(token);
	if(loc == NULL)
		serror(UNDEF_LAB); //label not defined
	else{
		gpush(prog);//save place to return to
		prog = loc; //start program running at that loc
	}
}

//Return from GOSUB
void greturn(){
	prog = gpop();
}

//Push GOSUB stack
void gpush(char *s){
	if(gtos == SUB_NEST)
		serror(TOO_MNY_GOSUB);
	gstack[gtos] = s;
	gtos++;
}

//POP GOSUB stack
char *gpop(){
	if(gtos == 0)
		serror(RET_WO_GOSUB);
	gtos--;
	return(gstack[gtos]);
}

