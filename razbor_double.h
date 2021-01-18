#ifndef _RAZBOR_DOUBLE_
#define _RAZBOR_DOUBLE_

void eval_exp(double &result);
void eval_exp1(double &result);
void eval_exp2(double &result);
void eval_exp3(double &result);
void eval_exp4(double &result);
void eval_exp5(double &result);
void eval_exp6(double &result);
void atom(double &result);

void exec_sin(double& result);
void exec_cos(double& result);
void exec_tan(double& result);
void exec_log(double& result);
void exec_exp(double& result);
void exec_exp(double& result);
void exec_abs(double& result);
void exec_atn(double& result);
void exec_sqr(double& result);


//void get_token();
int get_token();
void  putback();

//---------------------------------------------------///

void serror (int error);
int look_up(char *s);
int isdelim(char c);
int is_sp_tab(char c);
double find_var(char *s);


#endif