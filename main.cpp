//CopyRight by Librian
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <fstream>
#include <vector>
#include "lexical.h"
#include "initial.h"
#include "grammatical.h"
#include "tacode.h"
#include "assemble.h"


#define FILENAME "code.txt" //Filename defination
//used when scan declarations to fill symbol table
#define INTDCL 1
#define CHARDCL 2
#define VOIDCL 3


using namespace std;
///Some Const Defination

const string reserve_word[RWAMT] = { "case", "char", "const", "else", "if",
"int", "main", "printf", "return",
"scanf", "switch", "void", "while" };
const int in_to_sym[RWAMT] = { CASETK, CHARTK, CONSTTK, ELSETK, IFTK, INTTK, MAINTK, PRINTFTK, RETURNTK, SCANFTK, SWITCHTK, VOIDTK, WHILETK };

///File Defination
FILE *fin;
int fopen_res=fopen_s(&fin, FILENAME, "r");

///Records while Lexical
int sym;
//int cpoint=0;//point to the position of source code
int cline = 1;//line of source code
char ch;//next character
string iden;//last identifier
string str;//last string
int num;//last number==>also record index of constant in reserve_word[]

///Global Variables while Grammatical
symList symTab;//Symbol Table
symList* symPos = &symTab;//Positive Symbol Table.
varList* funcVarList;
bool global=true;
//Global variables while Generate ThreeAdressCode.
vector<TAcode> tacList;
Generation gen=Generation();

/****ToolFunction****/
void nextSym();
int bi_search(string);//binary search constants in reserve_word[]
bool is_let(char);//return true if input is a letter
bool is_num(char);//return true if input is a number
string delUnderLine(string);
string my_itoa(int);
int my_atoi(string);
//void outputTac(TAcode, ofstream);
/****FunctionInAssemble****/
void getAssemStack(ofstream& fout);
void getAssemCode(ofstream& fout);
varSrc getSrc(string, tmpVarList&, vector<paramList>*,varList*);
void movMem2Reg(ofstream&, varSrc, string, tmpVarList&, vector<paramList>* , varList*, string,bool=false);
void movReg2Mem(ofstream&, varSrc, string, tmpVarList&, vector<paramList>* , varList*);
/****Subprogram****/
//ResBise bise_symTab(string);//return -1 if input exists in symTab,return i+1 which symTab[i].name<input but symTab[i+1].name>input.
void _procedure();//程序
void _const_dcl();//常量说明
void _dcl_head(int*);//声明头部
bool _var_dcl(int*);//变量说明--如果正常结束返回true，如果在<声明头部>判断不再是变量说明，则返回0（但不报错）.
void _func_def_tail(int);//函数定义尾部

void _var_def_tail(int);//变量定义尾部
void _main_func();//主函数
void _const_def();//常量定义
void _unsigned_int();//无符号整数
void _signed_int();//整数

void _const();//常量
void _param_list(int);//参数表
void _comp_statements();//复合语句
void  _statements_list();//语句列
void _expression(string);//表达式

void _term(string);//项
void _factor(string res);//因子
void _func_call_tail(bool);//函数调用尾部
void _statement();//语句
void _conditional_statement();//条件语句

void _loop_statement();//循环语句
void _assignment_statement_tail();//赋值语句尾部
void _scanf_statement();//读语句
void _printf_statement();//写语句
void _switch_statement();//情况语句

void _return_statement();//返回语句
void _condition(string);//条件
void _condition_list(string);//情况表
void _condition_substatement();//情况子语句
void _call_param_list();//值参数表

void error(int, string info="");

int main()
{
	ofstream fout,fout2;
	fout.open(" tac.txt");
	fout2.open("E:\\MyProjects\\X86\\res.asm");
	ch = fgetc(fin);
	nextSym();
	_procedure();
	for (int i = 0; i < tacList.size(); i++)tacList[i].output(fout);
	symTab.getAssemData(fout2);
	fout << endl;
	//getAssemStack(fout2);
	fout << endl;
	getAssemCode(fout2);
	fclose(fin);
	fout.close();
	fout2.close();

	return 0;
}


/****To_Ol Functions****/

int bi_search(string s)// return index if s in reseave_word[], return -1 if not.
{
	int beg = 0, sto = RWAMT - 1;
	int mid;
	while (beg<sto){
		mid = (beg + sto) / 2;
		if (s<reserve_word[mid])
			sto = mid - 1;
		else if (s>reserve_word[mid])
			beg = mid + 1;
		else return mid;
	}
	if (s==reserve_word[beg])return beg;
	return -1;
}

bool is_let(char in)
{
	if (in >= 'a'&&in <= 'z')return true;
	if (in >= 'A'&&in <= 'Z')return true;
	return false;
}
bool is_num(char in)
{
	if (in >= '0'&&in <= '9')return true;
	return false;
}
string delUnderLine(string s)
{
	string res = "";
	for (int j = 0; j < s.size(); j++){
		if (s[j] != '_')res += s[j];
	}
	return res;
}
void nextSym()
{
	while (true){
		int i;
		if (is_let(ch)){
			iden = string(1,ch);
			for (i = 1; i<IDMAX + 1; i++){
				ch=fgetc(fin);
				if (is_let(ch) || is_num(ch))
					iden += ch;
				else break;
			}
			if (i == IDMAX + 1)
				error(ERROR_TOOLONGIDEN,"Too long indentifier.");
			num = bi_search(iden);
			sym = (num == -1) ? 1 : in_to_sym[num];
			return;
		}
		else if (is_num(ch)){
			num = ch - '0';
			while (true){
				ch=fgetc(fin);
				if (is_num(ch))
					num = num * 10 + (ch - '0');
				else break;
				if (num<0)
					error(ERROR_NUMOVERFLOW,"Number over flow.");
			}
			sym=INTCON;
			return;
		}
		else if (ch == '\''){
			ch=fgetc(fin);
			num = ch;
			ch=fgetc(fin);
			if (ch != '\'')
				error(ERROR_TOOMANYCINSQ,"One character required in SINGLE quote mark.");
			ch=fgetc(fin);
			sym=CHARCON;
			return;
		}
		else if (ch == '\"'){
			str = string("");
			ch=fgetc(fin);
			for (i = 0; i<STRMAX + 1; i++){
				if (ch == 34){
					ch=fgetc(fin);
					sym=STRCON;
					return;
				}
				else if (ch >= 32 && ch <= 126){
					str += ch;
					ch=fgetc(fin);
				}
				else error(ERROR_INVLDCINSTR,"Invalid character occured in string.");
			}
			if (i == STRMAX + 1)error(ERROR_TOOLONGSTR,"Too long string.");
		}
		else if (ch == '<'){
			ch=fgetc(fin);
			if (ch == '='){ ch=fgetc(fin); sym = LEQ; }
			else sym=LSS;
			return;
		}
		else if (ch == '>'){
			ch=fgetc(fin);
			if (ch == '='){ ch=fgetc(fin); sym = GEQ; }
			else sym=GRE;
			return;
		}
		else if (ch == '='){
			ch=fgetc(fin);
			if (ch == '='){ ch=fgetc(fin); sym = EQL; }
			else sym= ASSIGN;
			return;
		}
		else if (ch == '!'){
			ch=fgetc(fin);
			if (ch != '=')error(ERROR_INVLDEXC,"Invalid EXCLAMATORY mark.");
			ch=fgetc(fin);
			sym=NEQ;
			return;
		}
		else if (ch == '+'){ ch=fgetc(fin); sym = PLUS; return; }
		else if (ch == '-'){ ch=fgetc(fin); sym = MINU; return; }
		else if (ch == '*'){ ch=fgetc(fin); sym = MULT; return; }
		else if (ch == '/'){ ch=fgetc(fin); sym = DIV; return; }
		else if (ch == ';'){ ch=fgetc(fin); sym = SEMICN; return; }
		else if (ch == ','){ ch=fgetc(fin); sym = COMMA; return; }
		else if (ch == ':'){ ch=fgetc(fin); sym = COLON; return; }
		else if (ch == '('){ ch=fgetc(fin); sym = LPARENT; return; }
		else if (ch == ')'){ ch=fgetc(fin); sym = RPARENT; return; }
		else if (ch == '['){ ch=fgetc(fin); sym = LBRACK; return; }
		else if (ch == ']'){ ch=fgetc(fin); sym=RBRACK; return; }
		else if (ch == '{'){ ch=fgetc(fin); sym = LBRACE; return; }
		else if (ch == '}'){ ch=fgetc(fin); sym = RBRACE; return; }
		else if (ch == ' ' || ch == '\t' || ch == '\v' || ch == '\r')ch=fgetc(fin);
		else if (ch == '\n'){ cline++; cout << "Entering Line." << cline<<endl; ch = fgetc(fin); }
		else if (ch == EOF){sym = EOF; return;}
		else error(ERROR_INVLDCINCD,"Invalid character in code.");
	}
}
/****FunctionInAssemble****/
void getAssemStack(ofstream& fout)
{
	fout << ".STACK" << endl;
	fout << "STACKS DW " << STACKDEP << "DUP(?)" << endl;

}
/**********************************/
/*offset: record the total offset relative to BP*/
/*tvl : temporary variable List.*/
/*funame: name of current name*/
/*loVarList: pointer to the local variableList of current function.*/
/**********************************/
void getAssemCode(ofstream& fout)
{
	int offset=0;
	tmpVarList tvl = tmpVarList(); 
	string funame;
	varList* loVarList=NULL;
	vector<paramList>* parList=NULL;
	fout << ".CODE" << endl;

	for (int i = 0; i < tacList.size(); i++){
		fout << "\n;";
		tacList[i].output(fout);
		if (tacList[i].op==TANOP&&tacList[i].flag.substr(0, 10) == "@flag_func"){
			funame = tacList[i].flag.substr(11);
			int funcindex = symTab.lookup(funame);
			loVarList = symTab.list[funcindex].funcsym;
			parList = &(symTab.list[funcindex].parlist);
			int j = i;
			while (tacList[j].op != TAFEND){
				if (tacList[j].arg1.substr(0, 4) == "@tmp")tvl.insert(tacList[j].arg1);
				if (tacList[j].arg2.substr(0, 4) == "@tmp")tvl.insert(tacList[j].arg2);
				if (tacList[j].res.substr(0, 4) == "@tmp")tvl.insert(tacList[j].res);
				j++;
			}

			fout << "FUNC" << funame << " PROC" << endl;

			fout << "PUSH EBP" << endl;
			fout << "MOV EBP,ESP" << endl;
			fout << "XOR EAX,EAX" << endl;

			for (vector<varSym>::iterator it = loVarList->list.begin(); it != loVarList->list.end();it++){
				switch (it->type)
				{
				case CONSCHAR:
				case CONSINT:fout << "CON" << funame << it->name << " EQU " << it->value << 'd' << endl; break;
				case VARCHAR:fout << "PUSH EAX" << endl; offset -= 4; it->offset = offset; break;
				case VARINT:fout << "PUSH EAX" << endl; offset -= 4; it->offset = offset; break;
				case CHARRAY:for (int k = 0; k < it->value; k++){ fout << "PUSH EAX" << endl; offset -= 1; }it->offset = offset; break;
				case INTARRAY:for (int k = 0; k < it->value; k++){ fout << "PUSH EAX" << endl; offset -= 1; }it->offset = offset; break;
				}
			}
			for (vector<tmpVarPos>::iterator it = tvl.list.begin(); it != tvl.list.end();it++){
				fout << "PUSH EAX" << endl;
				offset -= 4;
				it->offset = offset;
			}
			continue;
		}
		if (tacList[i].op == TASET){
			varSrc res = getSrc(tacList[i].res, tvl, parList, loVarList);
			fout << "MOV EAX," << tacList[i].arg1 << endl;
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TANOP){
			fout << delUnderLine(tacList[i].flag.substr(1)) << ':' << endl;
		}
		if (tacList[i].op == TAMOV){
			varSrc res;
			if (tacList[i].arg1 != "@returnReg"){
				res = getSrc(tacList[i].arg1, tvl, parList, loVarList);

				movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			}
			if (tacList[i].res == "@returnReg")continue;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TAADD){
			varSrc res = getSrc(tacList[i].arg1, tvl,parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "ADD EAX,EBX" << endl;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TAMINU){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "SUB EAX,EBX" << endl;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TAMULT){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "IMUL EAX,EBX" << endl;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TADIV){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "IDIV EBX" << endl;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TANEG){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList,funame);
			fout << "NEG EAX" << endl;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);
			continue;
		}
		if (tacList[i].op == TAARIGHT){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame,true);
			res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "SAL EBX,2d" << endl;
			fout << "ADD EAX,EBX" << endl;
			fout << "MOV EAX,[EAX]" << endl;
			res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movReg2Mem(fout, res, "EAX", tvl, parList, loVarList);

			continue;
		}
		if (tacList[i].op == TAARRLEFT){
			varSrc res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame,true);
			res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "SAL EBX,2d" << endl;
			fout << "ADD EAX,EBX" << endl;
			res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			fout << "MOV [EAX],EBX" << endl;
			continue;
		}
		if (tacList[i].op == TAJMP){
			
			fout << "JMP " << delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		
		if (tacList[i].op == TAJGT){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			if( (tacList[i].arg2[0] >= '0'&&tacList[i].arg2[0] <= '9')||tacList[i].arg2[0]=='-'){
				fout << "MOV EBX," << my_atoi(tacList[i].arg2) << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			}
			fout << "CMP EAX,EBX" << endl;
			fout << "JG " <<delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		if (tacList[i].op == TAJGE){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			if ((tacList[i].arg2[0] >= '0'&&tacList[i].arg2[0] <= '9') || tacList[i].arg2[0] == '-'){
				fout << "MOV EBX," << my_atoi(tacList[i].arg2) << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			}
			fout << "CMP EAX,EBX" << endl;
			fout << "JGE " << delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		if (tacList[i].op == TAJLT){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			if ((tacList[i].arg2[0] >= '0'&&tacList[i].arg2[0] <= '9') || tacList[i].arg2[0] == '-'){
				fout << "MOV EBX," << my_atoi(tacList[i].arg2) << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			}
			fout << "CMP EAX,EBX" << endl;
			fout << "JL " << delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		if (tacList[i].op == TAJLE){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			if ((tacList[i].arg2[0] >= '0'&&tacList[i].arg2[0] <= '9') || tacList[i].arg2[0] == '-'){
				fout << "MOV EBX," << my_atoi(tacList[i].arg2) << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			}
			fout << "CMP EAX,EBX" << endl;
			fout << "JLE " << delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		if (tacList[i].op == TAJEQL){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			if ((tacList[i].arg2[0] >= '0'&&tacList[i].arg2[0] <= '9') || tacList[i].arg2[0] == '-'){
				fout << "MOV EBX," << my_atoi(tacList[i].arg2) << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			}
			fout << "CMP EAX,EBX" << endl;
			fout << "JE " << delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		if (tacList[i].op == TAJNE){
			varSrc res = getSrc(tacList[i].arg1, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			if ((tacList[i].arg2[0] >= '0'&&tacList[i].arg2[0] <= '9') || tacList[i].arg2[0] == '-'){
				fout << "MOV EBX," << my_atoi(tacList[i].arg2) << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EBX", tvl, parList, loVarList, funame);
			}
			fout << "CMP EAX,EBX" << endl;
			
			fout << "JNE " << delUnderLine(tacList[i].res.substr(1)) << endl;
			continue;
		}
		if (tacList[i].op == TACALL){
			fout << "CALL FUNC" << tacList[i].res << endl;
			continue;
		}
		if (tacList[i].op == TARETURN){
			fout << "MOV ESP,EBP" << endl;
			fout << "POP EBP" << endl;
			//if (funame == "main"){
			//	fout << "MOV AH,1" << endl;
			//	fout << "INT 21H" << endl;
			//}
			fout << "RET " << 4 * (parList->size()) << endl;
			continue;
		}
		if (tacList[i].op == TAPUSH){
			varSrc res = getSrc(tacList[i].res, tvl, parList, loVarList);
			movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
			fout << "PUSH EAX" << endl;
			continue;
		}
		if (tacList[i].op == TASCANF){
			varSrc res = getSrc(tacList[i].res, tvl, parList, loVarList);
			fout << "MOV EAX,EBP" << endl;
			fout << "ADD EAX,";
			switch (res.src){
			case TMPVAR:fout << tvl.list[res.index].offset ; break;
			case LOCVAR: fout << loVarList->list[res.index].offset ; break;
			case GLOVAR:fout <<  symTab.list[res.index].name; break;
			case PARVAR:fout << 4 * parList->size() - 4 * res.index + 2; break;
			}
			fout << endl;
			fout << "invoke crt_scanf,CTXT(\"%d\"),EAX" << endl;
			continue;
		}
		if (tacList[i].op == TAPRINTF){
			varSrc res; 
			if (tacList[i].arg2== ""){
				fout << "XOR EAX,EAX" << endl;
				fout << "invoke crt_printf,CTXT(\"" << tacList[i].arg1 << "\",EAX" << endl;
			}
			else if (tacList[i].arg1 == ""){
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
				fout << "invoke crt_printf,CTXT(\"%d\"),EAX" << endl;
			}
			else {
				res = getSrc(tacList[i].arg2, tvl, parList, loVarList);
				movMem2Reg(fout, res, "EAX", tvl, parList, loVarList, funame);
				fout << "invoke crt_printf,CTXT(\"" << tacList[i].arg1 << "\"),EAX" << endl;
			}
			continue;
		}
		if (tacList[i].op == TAFEND){
			tvl.clear();
			offset = 0;

			fout << "FUNC" << funame << " ENDP" << endl;
			continue;
		}
	}
	fout << "END FUNCmain" << endl;
}
void movMem2Reg(ofstream& fout, varSrc res, string reg, tmpVarList& tvl, vector<paramList>* parList,varList* loVarList,string funame,bool offset)
{
	fout << "MOV " << reg << ',';
	switch (res.src)
	{
	case TMPVAR:fout << "[EBP][" << tvl.list[res.index].offset << "]" << endl; break;
	case LOCVAR: if (loVarList->list[res.index].type == CONSINT || loVarList->list[res.index].type == CONSCHAR)fout << "CON" << funame << loVarList->list[res.index].name << endl;
				 else if (offset)fout << loVarList->list[res.index].offset << endl;else fout << "[EBP][" << loVarList->list[res.index].offset << "]" << endl; break;
	case GLOVAR:if (symTab.list[res.index].type == CONSINT || symTab.list[res.index].type == CONSCHAR)fout << "CONGLOBAL" << symTab.list[res.index].name << endl;
				else if (offset)fout << "OFFSET VAR" << symTab.list[res.index].name << endl; else fout << "VAR" << symTab.list[res.index].name << endl; break;
	case PARVAR:fout << "[EBP][" << 4 * parList->size() - 4 * res.index + 4 << "]" << endl;
	}
}
void movReg2Mem(ofstream& fout, varSrc res, string reg, tmpVarList& tvl, vector<paramList>* parList, varList* loVarList)
{
	switch (res.src){
	case TMPVAR:fout << "MOV [EBP][" << tvl.list[res.index].offset << "]"; break;
	case LOCVAR: fout << "MOV [EBP][" << loVarList->list[res.index].offset << "]"; break;
	case GLOVAR:fout << "MOV VAR" << symTab.list[res.index].name; break;
	case PARVAR:fout << "MOV [EBP][" << 4 * parList->size() - 4 * res.index + 2 << "]" << endl;
	}
	fout << "," <<reg<< endl;
}
varSrc getSrc(string name, tmpVarList& tvl,vector<paramList>* parList, varList* loVarList)
{
	int i;
	if (name[0]== '@'){
		for (i = 0; i < tvl.list.size(); i++)
		if (tvl.list[i].name == name)break;
		return{ TMPVAR, i };
	}
	for (i = 0; i < parList->size(); i++){
		if ((*parList)[i].name == name)return{ PARVAR, i };
	}
	i = loVarList->lookup(name);
	if (i < 0)return{ GLOVAR, symTab.lookup(name) };
	else return{ LOCVAR, i };
}
/****Sub Programs used in Grammatical Analysis****/
void _procedure()
{
	int type;
	bool res_var_dcl;//result of function _var_dcl()
	if (sym==CONSTTK)
		_const_dcl();

	if (sym == INTTK || sym == CHARTK){
		_dcl_head(&type);
		if (sym == LBRACK || sym == COMMA || sym == SEMICN){
			res_var_dcl = _var_dcl(&type);
			if (!res_var_dcl)_func_def_tail(type);
		}
		else if (sym == LPARENT)
			_func_def_tail(type);
		else error(ERROR_GRAM);
	}

	while (true){
		if (sym == INTTK||sym==CHARTK){
			_dcl_head(&type);
			_func_def_tail(type);
			continue;
		}
		else if (sym == VOIDTK){
			nextSym();
			if (sym == IDEN){
				nextSym();
				_func_def_tail(type);
			}
			else if (sym == MAINTK){
				_main_func();
				break;
			}
			else error(ERROR_GRAM);
		}
	}
	if (sym == EOF)return;
	else error(ERROR_GRAM,"Invalid codes after main function.");
}

void _const_dcl()
{
	if (sym == CONSTTK)
		nextSym();
	else error(ERROR_GRAM);

	_const_def();
	if (sym != SEMICN)error(ERROR_GRAM);
	nextSym();
	while (sym == CONSTTK){
		nextSym();
		_const_def();
		if (sym != SEMICN)error(ERROR_GRAM);
		nextSym();
	}
	return;
}

void _const_def()
{
	int type;
	if (sym == INTTK){
		nextSym();
		if (sym != IDEN)error(ERROR_GRAM);
		nextSym();
		if (sym != ASSIGN)error(ERROR_GRAM);
		nextSym();
		_signed_int();
		if (global){ if (symPos->insert(iden, CONSINT, cline, num) < 0)error(ERROR_IDENREDCL, "Const Redeclaration. "); }
		else { if(funcVarList->insert(iden, CONSINT, cline, num)<0)error(ERROR_IDENREDCL,"COnst Redeclaration"); }

		while (sym == COMMA){
			nextSym();
			if (sym != IDEN)error(ERROR_GRAM);
			nextSym();
			if (sym != ASSIGN)error(ERROR_GRAM);
			nextSym();
			_signed_int();
			if (global){ if (symPos->insert(iden, CONSINT, cline, num) < 0)error(ERROR_IDENREDCL, "Const Redeclaration. "); }
			else { if (funcVarList->insert(iden, CONSINT, cline, num)<0)error(ERROR_IDENREDCL, "COnst Redeclaration"); }
		}
	}
	else if (sym == CHARTK){
		nextSym();
		if (sym != IDEN)error(ERROR_GRAM);
		nextSym();
		if (sym != ASSIGN)error(ERROR_GRAM);
		nextSym();
		if (sym != CHARCON)error(ERROR_GRAM);
		nextSym();
		if (global){ if (symPos->insert(iden, CONSCHAR, cline, num) < 0)error(ERROR_IDENREDCL, "Const Redeclaration. "); }
		else { if (funcVarList->insert(iden, CONSCHAR, cline, num)<0)error(ERROR_IDENREDCL, "COnst Redeclaration"); }
		while (sym == COMMA){
			nextSym();
			if (sym != IDEN)error(ERROR_GRAM);
			nextSym();
			if (sym != ASSIGN)error(ERROR_GRAM);
			nextSym();
			if (sym != CHARCON)error(ERROR_GRAM);
			nextSym();
			if (global){ if (symPos->insert(iden, CONSCHAR, cline, num) < 0)error(ERROR_IDENREDCL, "Const Redeclaration. "); }
			else { if (funcVarList->insert(iden, CONSCHAR, cline, num)<0)error(ERROR_IDENREDCL, "COnst Redeclaration"); }
		}
	}
	else error(ERROR_GRAM);
}

void _unsigned_int()
{
	if (sym != INTCON)error(ERROR_GRAM,"Integer required");
	if (num == 0)error(ERROR_NZIBUTZERO,"NonZero integer required but zero given");
	nextSym();
}

void _signed_int()
{
	if (sym == INTCON&&num == 0){
		nextSym();
		return;
	}
	if (sym == PLUS){
		nextSym();
		_unsigned_int();
	}
	else if (sym == MINU){
		nextSym();
		_unsigned_int();

	}
	else _unsigned_int();
}

void _dcl_head(int* type)
{
	if (sym == INTTK) *type = INTDCL;
	else if (sym == CHARTK)*type = CHARDCL;
	else error(ERROR_GRAM, "in _dcl_head");
	nextSym();
	if (sym != IDEN)error(ERROR_GRAM, "Invalid identifier when declaration");
	nextSym();
}

bool _var_dcl(int* type)
{
	_var_def_tail(*type);
	if (sym != SEMICN)error(ERROR_GRAM, "required SEMICN after variables defination.");
	nextSym();
	while (sym == INTTK || sym == CHARTK){
		_dcl_head(type);
		if (sym != LBRACK&&sym != COMMA&&sym != SEMICN)return false;
		_var_def_tail(*type);
		if (sym != SEMICN)error(ERROR_GRAM, "required SEMICOLON after variable defination.");
		nextSym();
	}
	return true;
}

void _var_def_tail(int type)
{
	if (sym == LBRACK){
		nextSym();
		_unsigned_int();
		if (sym != RBRACK)error(ERROR_GRAM, "Missing Right BRACK.");
		nextSym();
		if (global){ if (symPos->insert(iden, (type == INTDCL ? INTARRAY : CHARRAY), cline, num) < 0)error(ERROR_IDENREDCL, "Redeclaration-Array."); }
		else if (funcVarList->insert(iden, (type == INTDCL ? INTARRAY : CHARRAY), cline, num) < 0)error(ERROR_IDENREDCL, "Redeclaration-Array.");
	}
	else {
		if (global){ if (symPos->insert(iden, (type == INTDCL ? VARINT : VARCHAR), cline) < 0)error(ERROR_IDENREDCL, "Redeclaration-VARINT."); }
		else if (funcVarList->insert(iden, (type == INTDCL ? VARINT : VARCHAR), cline) < 0)error(ERROR_IDENREDCL, "Redeclaration-VARINT.");
	}

	while (sym == COMMA){
		nextSym();
		if (sym != IDEN)error(ERROR_GRAM, "Illegal identifier name.");
		nextSym();
		if (sym == LBRACK){
			nextSym();
			_unsigned_int();
			if (sym != RBRACK)error(ERROR_GRAM, "Missing Right BRACK.");
			nextSym();
			if (global){ if (symPos->insert(iden, (type == INTDCL ? VARINT : VARCHAR), cline) < 0)error(ERROR_IDENREDCL, "Redeclaration-VARINT."); }
			else if (funcVarList->insert(iden, (type == INTDCL ? VARINT : VARCHAR), cline) < 0)error(ERROR_IDENREDCL, "Redeclaration-VARINT.");
		}
		else{
			if (global){ if (symPos->insert(iden, (type == INTDCL ? VARINT : VARCHAR), cline) < 0)error(ERROR_IDENREDCL, "Redeclaration-VARINT."); }
			else if (funcVarList->insert(iden, (type == INTDCL ? VARINT : VARCHAR), cline) < 0)error(ERROR_IDENREDCL, "Redeclaration-VARINT.");
		}
	}
}

void _const()
{
	if (sym == CHARCON){
		nextSym();
		return;
	}
	if (sym == PLUS || sym == MINU||sym==INTCON){
		_signed_int();
		return;
	}
}

void _func_def_tail(int type)
{
	int index;//index in symTable
	if (type == INTDCL)index = symPos->insert(iden, FUNCINT, cline, 0);
	else if (type == CHARDCL)index = symPos->insert(iden, FUNCHAR, cline, 0);
	else index = symPos->insert(iden, FUNCVOID, cline, 0);
	if (index < 0)error(ERROR_IDENREDCL, "Redeclarationl->Function.");

	if (sym != LPARENT)error(ERROR_GRAM,"required LPARENT after identifier when declare function.");
	tacList.push_back(TAcode(TANOP, "", "", "", gen.funcFlagGen(iden)));
	nextSym();
	_param_list(index);
	if (sym != RPARENT)error(ERROR_GRAM, "Missing Right PARENT.");
	nextSym();
	if (sym != LBRACE)error(ERROR_GRAM, "required Left BRACE when declare function.");
	global = false;
	funcVarList = symPos->list[index].funcsym;
	nextSym();
	
	_comp_statements();
	if (tacList[tacList.size() - 1].op != TARETURN)tacList.push_back(TAcode(TARETURN, "", "", ""));
	if (sym != RBRACE)error(ERROR_GRAM, "Missing Right BRACE.");
	global = true;
	tacList.push_back(TAcode(TAFEND, "", "", ""));
	nextSym();
}

void _comp_statements()
{
	int type;
	if (sym == CONSTTK)_const_dcl();
	if (sym == INTTK || sym == CHARTK){
		_dcl_head(&type);
		_var_dcl(&type);
	}
	_statements_list();
}

void _param_list(int index)
{
	int type;
	if (sym == INTTK || sym == CHARTK){
		type = (sym == INTTK ? VARINT : VARCHAR);
		nextSym();
		if (sym != IDEN)error(ERROR_GRAM, "Invalid PARAM Name.");
		if(!symPos->list[index].addParam(type, iden, cline))error(ERROR_IDENREDCL,"Redeclaration->Parameter.");
		nextSym();
		while (sym == COMMA){
			nextSym();
			if (sym != INTTK&&sym != CHARTK)error(ERROR_GRAM, "Invalid PARAM Form.");
			type = (sym == INTTK ? VARINT : VARCHAR);
			nextSym();
			if (sym != IDEN)error(ERROR_GRAM, "Invalid PARAM Name.");
			if (!symPos->list[index].addParam(type, iden, cline))error(ERROR_IDENREDCL, "Redeclaration->Parameter.");
			nextSym();
		}
	}
}

void _main_func()
{
	if (sym != MAINTK)error(ERROR_GRAM);
	nextSym();
	if (sym != LPARENT)error(ERROR_GRAM, "requ ired LeftPARENT after \"main\".");
	nextSym();
	if (sym != RPARENT)error(ERROR_GRAM, "Missing RightPARENT.");
	tacList.push_back(TAcode(TANOP, "", "", "", gen.funcFlagGen("main")));
	nextSym();
	int index=symTab.insert("main", FUNCVOID, cline);
	funcVarList = symPos->list[index].funcsym;
	if (sym != LBRACE)error(ERROR_GRAM, "required LeftBRACE after \"main()\"");
	global = false;
	nextSym();
	_comp_statements();
	global = true;
	if (tacList[tacList.size() - 1].op != TARETURN)tacList.push_back(TAcode(TARETURN, "", "", ""));
	if (sym != RBRACE)error(ERROR_GRAM, "Missing RightBRACE.");
	tacList.push_back(TAcode(TAFEND, "", "", ""));
	nextSym();
}

void _expression(string res){
	bool plusop=true;
	string exp = gen.tmpNameGen();
	if (sym == MINU)plusop = false;
	if(sym==PLUS||sym==MINU)nextSym();
	_term(res);
	if (!plusop)tacList.push_back(TAcode(TANEG, res, "", res));
	while (sym == PLUS || sym == MINU){
		plusop = sym == PLUS ? true : false;
		nextSym();
		_term(exp);
		if (plusop)tacList.push_back(TAcode(TAADD, res, exp,res));
		else tacList.push_back(TAcode(TAMINU, res, exp, res));
	}
}

void _term(string res)
{
	string exp = gen.tmpNameGen();
	bool mulop;
	_factor(res);
	while (sym == MULT || sym == DIV){
		mulop = sym == MULT ? true: false;
		nextSym();
		_factor(exp);
		if (mulop)tacList.push_back(TAcode(TAMULT, res, exp, res));
		else tacList.push_back(TAcode(TADIV, res, exp, res));
	}
}

void _factor(string res)
{
	if (sym == IDEN){
		string identmp = iden;
		nextSym();
		if (sym == LBRACK){
			string exp = gen.tmpNameGen();
			nextSym();
			_expression(exp);
			if (sym != RBRACK)error(ERROR_GRAM, "required LBRACK.");
			tacList.push_back(TAcode(TAARIGHT, identmp, exp, res));
			nextSym();
		}
		else if (sym == LPARENT){
			_func_call_tail(true);
			tacList.push_back(TAcode(TAMOV, "@returnReg","", res));
		}
		else tacList.push_back(TAcode(TAMOV, identmp, "", res));
		return;
	}
	else if (sym == INTCON){
		tacList.push_back(TAcode(TASET, my_itoa(num), "", res));
		nextSym();
		return;
	}
	else if (sym == CHARCON){
		tacList.push_back(TAcode(TASET, my_itoa(num), "", res));
		nextSym();
		return;
	}
	else if (sym == LPARENT){
		nextSym();
		_expression(res);
		if (sym != RPARENT)error(ERROR_GRAM, "required RPARENT.");
		nextSym();
		return;
	}
	else error(ERROR_GRAM, "WrongFuncCall:_factor().");
}

void _statement()
{
	if (sym == IFTK){
		_conditional_statement();
		return;
	}
	else if (sym == WHILETK){
		_loop_statement();
		return;
	}
	else if (sym == LBRACE){
		nextSym();
		_statements_list();
		if (sym != RBRACE)error(ERROR_GRAM, "required RBRACE.");
		nextSym();
		return;
	}
	else if (sym == IDEN){
		nextSym();
		if (sym == LPARENT){
			_func_call_tail(false);
			if (sym != SEMICN)error(ERROR_GRAM, "required SEMICN.");
			nextSym();
			return;
		}
		else if (sym == ASSIGN||sym==LBRACK){
			_assignment_statement_tail();
			if (sym != SEMICN)error(ERROR_GRAM, "required SEMICN.");
			nextSym();
			return;
		}
		else error(ERROR_GRAM, "Identifier is NOT supposed.");
	}
	else if (sym == SCANFTK){
		_scanf_statement();
		if (sym != SEMICN)error(ERROR_GRAM, "required SEMICN");
		nextSym();
		return;
	}
	else if (sym == PRINTFTK){
		_printf_statement();
		if (sym != SEMICN)error(ERROR_GRAM, "required SEMICN");
		nextSym();
		return;
	}
	else if (sym == SEMICN){
		nextSym();
		return;
	}
	else if (sym == SWITCHTK){
		_switch_statement();
		return;
	}
	else if (sym == RETURNTK){
		_return_statement();
		if (sym != SEMICN)error(ERROR_GRAM, "required SEMICN");
		nextSym();
		return;
	}
	else error(ERROR_GRAM, "WrongFuncCall:_statement().");
}

void _assignment_statement_tail()
{
	string idenName = iden;
	void* symbol;
	bool local;
	int symIndex = funcVarList->lookup(idenName);
	if (symIndex < 0){
		local = false;
		symIndex = symTab.lookup(idenName);
		if (symIndex < 0)error(ERROR_UNDCLIDEN, "Undeclared Identifier.");
	}
	else local = true;

	if (sym == ASSIGN){
		if (local){ if (!funcVarList->list[symIndex].isVar())error(ERROR_INVLDIDUSD, "Invalid Identifier Used.Var Required."); }
		else if (!symTab.list[symIndex].isVar())error(ERROR_INVLDIDUSD, "VariableRequired on the left of ASSIGN.");
		string exp=gen.tmpNameGen();
		nextSym();
		_expression(exp);
		tacList.push_back(TAcode(TAMOV, exp, "", idenName));
		gen.refresh();
		return;
	}
	else if (sym == LBRACK){
		if (local){ if (!funcVarList->list[symIndex].isArray())error(ERROR_INVLDIDUSD, "Invalid Identifier Used.Var Required."); }
		else if (!symTab.list[symIndex].isArray())error(ERROR_INVLDIDUSD, "ArrayIdentifier on the left LBRACK.");
		string exp_index = gen.tmpNameGen(),exp_res=gen.tmpNameGen();
		nextSym();
		_expression(exp_index);
		if (sym != RBRACK)error(ERROR_GRAM, "required RBRACK.");
		nextSym();
		if (sym != ASSIGN)error(ERROR_GRAM, "Required ASSIGN.");
		nextSym();
		_expression(exp_res);
		tacList.push_back(TAcode(TAARRLEFT, exp_res, exp_index, idenName));
		gen.refresh();
		return;
	}
	else error(ERROR_GRAM, "Required ASSIGN.");
}

void _conditional_statement()
{
	string resTac;
	string flagif=gen.condFlagGen();
	string flagelse = gen.condFlagGen();
	if (sym != IFTK)error(ERROR_GRAM, "WrongFuncCall:_conditional_statement.");
	nextSym();
	if (sym != LPARENT)error(ERROR_GRAM, "LPARENT required after IFTK.");
	nextSym();
	_condition(flagelse);
	gen.refresh();
	if (sym != RPARENT)error(ERROR_GRAM, "RPARENT required.");
	nextSym();
	_statement();    
	gen.refresh();
	if (sym == ELSETK){
		tacList.push_back(TAcode(TAJMP, "", "", flagif));
		tacList.push_back(TAcode(TANOP, "", "", "", flagelse));
		nextSym();
		_statement();
		tacList.push_back(TAcode(TANOP, "", "", "", flagif));
	}
	else tacList.push_back(TAcode(TANOP, "", "", "", flagelse));
	gen.refresh();
	return;
}

void _condition(string flag)
{
	string lexp = gen.tmpNameGen();
	_expression(lexp);
	if (sym == LSS||sym==LEQ||sym==GRE||sym==GEQ||sym==EQL||sym==NEQ){
		int comop = sym;
		string rexp = gen.tmpNameGen();
		nextSym();
		_expression(rexp);
		switch (comop)
		{
		case LSS:tacList.push_back(TAcode(TAJGE, lexp, rexp, flag)); break;
		case LEQ:tacList.push_back(TAcode(TAJGT, lexp, rexp, flag)); break;
		case GRE:tacList.push_back(TAcode(TAJLE, lexp, rexp, flag)); break;
		case GEQ:tacList.push_back(TAcode(TAJLT, lexp, rexp, flag)); break;
		case EQL:tacList.push_back(TAcode(TAJNE, lexp, rexp, flag)); break;
		case NEQ:tacList.push_back(TAcode(TAJEQL, lexp, rexp, flag)); break;
		}
	}
	else tacList.push_back(TAcode(TAJEQL, lexp, "0", flag));
	return;
}

void _loop_statement()
{
	if (sym != WHILETK)error(ERROR_GRAM, "WrongFuncCall:_loop_statement.");
	string flagloop = gen.loopFlagGen();
	string flagbreak = gen.loopFlagGen();
	tacList.push_back(TAcode(TANOP, "", "", "", flagloop));
	nextSym();
	if (sym != LPARENT)error(ERROR_GRAM, "Required LPARENT after WHILETK");
	nextSym();
	_condition(flagbreak);
	gen.refresh();
	if (sym != RPARENT)error(ERROR_GRAM, "RPARENT required.");
	nextSym();
	_statement();
	gen.refresh();
	tacList.push_back(TAcode(TAJMP, "", "", flagloop));
	tacList.push_back(TAcode(TANOP, "", "", "", flagbreak));
	return;
}

void _switch_statement()
{
	if (sym != SWITCHTK)error(ERROR_GRAM, "WrongFuncCall:_switch_statement().");
	string exp = gen.tmpNameGen();
	nextSym();
	if (sym != LPARENT)error(ERROR_GRAM, "LPARENT required after SWITCHTK.");
	nextSym();
	_expression(exp);
	gen.refresh();
	if (sym != RPARENT)error(ERROR_GRAM, "RPARENT required.");
	nextSym();
	if (sym != LBRACE)error(ERROR_GRAM, "LBRACE required after switch().");
	nextSym();
	_condition_list(exp);
	if (sym != RBRACE)error(ERROR_GRAM, "RBRACE required.");
	nextSym();
	return;
}

void _condition_list(string exp)
{
	int jpos = tacList.size();
	while (sym == CASETK){
		if (sym != CASETK)error(ERROR_GRAM, "WrongFuncCall:_condition_substatement().");
		nextSym();
		_const();
		string flag = gen.swtFlagGen();
		tacList.insert(tacList.begin()+jpos,TAcode(TAJEQL, exp, my_itoa(num), flag));
		tacList.push_back(TAcode(TANOP, "", "", "", flag));
		if (sym != COLON)error(ERROR_GRAM, "Required COLON in case-statement.");
		nextSym();
		_statement();
		gen.refresh();

	}
		return;
}

//void _condition_substatement()
//{
//	if (sym != CASETK)error(ERROR_GRAM, "WrongFuncCall:_condition_substatement().");
//	nextSym();
//	_const();
//	if (sym != COLON)error(ERROR_GRAM, "Required COLON in case-statement.");
//	nextSym();
//	_statement();
//	return;
//}

void _func_call_tail(bool reFunc)
{
	string funcName = iden;
	int paraNum = 0, symIndex = symTab.lookup(iden);
	if (symIndex < 0)error(ERROR_UNDCLIDEN, "Function Undeclaration.");
	if (reFunc)if (!symTab.list[symIndex].isReFunc())error(ERROR_INVLDIDUSD, "Need RetFunction.");
	else if (!symTab.list[symIndex].isFunc())error(ERROR_INVLDIDUSD, "Need FUnction.");

	if (sym != LPARENT)error(ERROR_GRAM, "WrongFuncCall:_func_call_tail().");
	nextSym();
	if (sym == PLUS || sym == MINU || sym == IDEN || sym == INTCON || sym == CHARCON || sym == LPARENT){
		string exp = gen.tmpNameGen();
		_expression(exp);
		tacList.push_back(TAcode(TAPUSH, "", "", exp));
		gen.refresh();
		paraNum++;
		while (sym == COMMA){
			nextSym();
			exp = gen.tmpNameGen();
			_expression(exp);
			tacList.push_back(TAcode(TAPUSH, "", "", exp));
			paraNum++;
		}
	}
	if (sym != RPARENT)error(ERROR_GRAM, "Required RPARENT.");
	if (paraNum != symTab.list[symIndex].value)error(ERROR_UNMTCHPARA, "Unmatched parameter.");
	tacList.push_back(TAcode(TACALL, "", "", funcName));
	nextSym();
	return;
}

//void _call_param_list()
//{
//	if (sym == PLUS || sym == MINU || sym == IDEN || sym == INTCON || sym == CHARCON || sym == LPARENT){
//		_expression();
//		while (sym == COMMA){
//			nextSym();
//			_expression();
//		}
//	}
//	return;
//}

void _statements_list()
{
	while (sym == IFTK || sym == WHILETK || sym == LBRACE || sym == IDEN || sym == SCANFTK || sym == PRINTFTK || sym == SEMICN || sym == SWITCHTK || sym == RETURNTK){
		gen.refresh();
		_statement();
	}
	return;
}

void _scanf_statement()
{
	if (sym != SCANFTK)error(ERROR_GRAM, "WrongFuncCall:_scanf_statement().");
	nextSym();
	if (sym != LPARENT)error(ERROR_GRAM, "LPARENT required after SCANFTK.");
	nextSym();
	if (sym != IDEN)error(ERROR_GRAM, "IDENTIFIER required.");
	tacList.push_back(TAcode(TASCANF, "","",iden));
	nextSym();
	while (sym == COMMA){
		nextSym();
		if (sym != IDEN)error(ERROR_GRAM, "IDENTIFIER required.");
		tacList.push_back(TAcode(TASCANF, "", "", iden));
		nextSym();
	}
	if (sym != RPARENT)error(ERROR_GRAM, "RPARENT required.");
	nextSym();
	return;
}

void _printf_statement()
{
	if (sym != PRINTFTK)error(ERROR_GRAM, "WrongFuncCall:_printf_statement().");
	nextSym();
	if (sym != LPARENT)error(ERROR_GRAM, "LPARENT required after PRINTFTK.");
	string	exp = gen.tmpNameGen();
	nextSym();
	if (sym == STRCON){
		nextSym();
		if (sym == COMMA){
			nextSym();
			_expression(exp);
			tacList.push_back(TAcode(TAPRINTF, str, exp, ""));
		}
		else tacList.push_back(TAcode(TAPRINTF, str, "", ""));
	}
	else {
		_expression(exp);
		tacList.push_back(TAcode(TAPRINTF, "", exp, ""));
	}
	if (sym != RPARENT)error(ERROR_GRAM, "RPARENT required.");
	nextSym();
	return;
}

void _return_statement()
{
	if (sym != RETURNTK)error(ERROR_GRAM, "WrongFuncCall:_return_statement().");
	nextSym();
	if (sym == LPARENT){
		string exp = gen.tmpNameGen();
		nextSym();
		_expression(exp);
		if (sym != RPARENT)error(ERROR_GRAM, "Required RPARENT.");
		tacList.push_back(TAcode(TAMOV, exp,"", "@returnReg"));
		nextSym();
	}
	tacList.push_back(TAcode(TARETURN, "", "", ""));
	return;
}
void error(int n,string info)
{
	if (info=="")
		cout << "ERROR(" << n << ") in Line." << cline << endl;
	else cout << "ERROR(" << n << ") in Line." << cline << info << endl;
	exit(1);
}


/**ToolFunction**/
string my_itoa(int num)
{
	string temp;
	string res;
	bool neg = false;
	if (num == 0)return "0";
	if (num<0){
		neg = true;
		num = num*(-1);
	}
	while (num > 0){
		temp += (char)(num % 10 + '0');
		num /= 10;
	}
	if (neg)temp += '-';
	for (int i = temp.size() - 1; i >= 0; i--)res += temp[i];
	return res;
}

int my_atoi(string s)
{
	int res=0;
	bool neg = false;
	if (s[0] == '-'){
		neg = true;
		s = s.substr(1);
	}
	for (int i = 0; i < s.size(); i++)
		res = res * 10 + s[i] - '0';
	if (neg)return -1 * res;
	else return res;
}
