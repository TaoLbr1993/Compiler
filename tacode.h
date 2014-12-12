//ThreeAddressCode
#include <string>
#include <vector>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>

#define TANOP 0
#define TASET 1
#define TAADD 2
#define TAMINU 3
#define TAMULT 4
#define TADIV 5
#define TAARIGHT 6
#define TAARRLEFT 7
#define TAJMP 8
#define TAJGT 10
#define TAJGE 11
#define TAJLT 12
#define TAJLE 13
#define TAJEQL 14
#define TAJNE 15
#define TACALL 16
#define TARETURN 17
#define TAPUSH 18
#define TASCANF 19
#define TAPRINTF 20
#define TANEG 21
#define TAFEND 22
#define TAMOV 23

using namespace std;
class TAcode
{
public:
	int op;
	string arg1;
	string arg2;
	string res;
	string flag;
	TAcode(int op, string arg1, string arg2, string res, string flag = "") :op(op), arg1(arg1), arg2(arg2), res(res), flag(flag)
	{}
public:
	void output(ofstream& fout){
		string revTac[24] = { "OP", "SET", "ADD", "MINU", "MULT", "DIV", "ARIGHT", "ARRLEFT", "JMP", "9", "JGT", "JGE", "JLT", "JLE", "JEQL", "JNE", "CALL", "RETURN", "PUSH", "SCANF", "PRINTF", "NEG","FEND","MOV"};
		if (flag != "")fout << flag << ':' << endl;
		else fout <<"    "<< revTac[op] << "<>" << arg1 << "<>" << arg2<<"<>"<<res << endl;
	}
};

class Generation
{
public:
	int tmp;
	int cond;
	int loop;
	int swt;
	Generation() :tmp(0), cond(0), loop(0), swt(0)
	{}

	string tmpNameGen(){
		return "@tmp_" + itoa(++tmp);
	}
	string funcFlagGen(string name){
		return "@flag_func_" + name;
	}
	string condFlagGen(){
		return "@flag_cond_" + itoa(++cond);
	}
	string loopFlagGen(){
		return "@flag_loop_" + itoa(++loop);
	}
	string swtFlagGen(){
		return "@flag_swt_" + itoa(++swt);
	}
	void refresh(){
		tmp = 0;
	}
private:
	string itoa(int num)
	{
		string temp;
		string res;
		while (num > 0){
			temp += (char)(num % 10 + '0');
			num /= 10;
		}
		for (int i = temp.size()-1; i >= 0; i--)res += temp[i];
		return res;
	}
};

//CopyRight by Librian
