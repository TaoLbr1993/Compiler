#include <string>
#include <vector>
#include<iterator>
//Used in struct Symbol
#define CONSINT 1
#define CONSCHAR 2
#define FUNCINT 3
#define FUNCHAR 4
#define INTARRAY 5
#define CHARRAY 6
#define VARINT 7
#define VARCHAR 8
#define FUNCVOID 9

using namespace std;
//record the result of binary search in Symbol Table
typedef struct ResBise{
	bool exist;//if it is exit.
	int pos;//position
}ResBise;
class paramList{
public:
	int type;
	string name;
	paramList(int type, string name) :type(type), name(name)
	{}

};
class varSym
{
public:
	string name;
	int type;
	int staLine;//state Line
	int value;
	int offset;
	varSym(string name, int type, int staLine, int value = 0) :name(name), value(value), type(type), staLine(staLine), offset(-1)
	{}
	//void addQuoLine(int ql){
	//	this->quoLine.push_back(ql);
	//}
	void output()
	{
		cout << name << '#' << type << '#' << staLine << '#' << value << endl;
	}
	bool isVar(){
		if (type == VARINT || type == VARCHAR)return true;
		return false;
	}
	bool isArray(){
		if (type == INTARRAY || type == CHARRAY)return true;
		return false;
	}
	bool isConst(){
		if (type == CONSINT || type == CONSCHAR)return true;
		else return false;
	}
};
//Symbol Tables class
class varList
{
public:
	vector<varSym> list;
public:
	//return -1 if insertion failed,return insert position if insertion succeeded.
	int insert(string name, int type, int staLine, int value = 0)
	{
		ResBise res_bise = bise_symTab(name);
		if (res_bise.exist)return -1;

		varSym temp = varSym(name, type, staLine, value);
		this->list.insert(list.begin() + res_bise.pos, temp);
		return res_bise.pos;
	}
	//return -1 if name is not in table, return position if name exists in table.
	int lookup(string name)
	{
		ResBise res_bise = bise_symTab(name);
		if (res_bise.exist)return res_bise.pos;
		else return -1;
	}

	void output()
	{
		for (vector<varSym>::iterator it = list.begin(); it != list.end(); it++)
			it->output();
	}
private:
	/****Function for Grammatical Analysis****/
	////===>return -1 if input exists in symTab,return i+1 which symTab[i].name<input but symTab[i+1].name>input.
	ResBise bise_symTab(string name)
	{
		int beg = 0, sto = this->list.size() - 1;
		int mid;
		while (beg <= sto){
			mid = (beg + sto) / 2;
			if (name<this->list[mid].name)
				sto = mid - 1;
			else if (name>this->list[mid].name)
				beg = mid + 1;
			else return{ true, mid };
		}
		//if (name == reserve_word[beg])return -1;
		return{ false, beg };
	}
};

//struct of symbols in SymbolsTables 
class Symbol
{
public:
	string name;
	int type;
	int staLine;//state Line
	int value;
	int offset;//assemble use
	varList  *funcsym;
	vector<paramList>	parlist;

	Symbol(string name, int type, int staLine, int value = 0) :name(name), value(value), type(type), staLine(staLine), funcsym(NULL), offset(-1)
	{
		if (this->type == FUNCINT || this->type == FUNCHAR || this->type == FUNCVOID)
			this->funcsym = new varList();
		else this->funcsym = NULL;
	}
	//return false if insert failed,return true if insert succeed.
	bool addParam(int type, string name, int staLine)
	{
		for (int i = 0; i < parlist.size(); i++){
			if (parlist[i].name == name)return false;
		}
		parlist.push_back(paramList(type,name));
		value++;
		return true;
	}
	bool addVar(int type, string name, int staLine, int value = 0)
	{
		if (funcsym == NULL){
			cout << "FatalError:Insert VAR into VAR." << endl;
			return false;
		}
		if (!this->funcsym->insert(name, type, staLine, value))return false;
		return true;
	}

	bool isReFunc(){
		if (this->type == FUNCINT || this->type == FUNCHAR)return true;
		return false;
	}
	bool isFunc(){
		if (isReFunc()||this->type == FUNCVOID)return true;
		return false;
	}
	bool isVar(){
		if (this->type == VARINT || this->type == VARCHAR)return true;
		return false;
	}
	bool isArray(){
		if (this->type == INTARRAY || this->type == CHARRAY)return true;
		return false;
	}
	//void addQuoLine(int ql){
	//	this->quoLine.push_back(ql);
	//}
	void output()
	{
		cout << name << '#' << type << '#' << staLine << '#' << value << endl;
		if (funcsym != NULL){
			cout << "==============================" << endl;
			for (vector<paramList>::iterator it = parlist.begin(); it != parlist.end(); it++)
				cout << it->name << ' ';
			cout << "==============================" << endl;
			funcsym->output();

		}
	}
};
//Symbol Tables class
class symList
{
public:
	vector<Symbol> list;
public:
	//return -1 if insertion failed,return insert position if insertion succeeded.
	int insert(string name,int type,int staLine,int value=0)
	{
		ResBise res_bise=bise_symTab(name);
		if (res_bise.exist)return -1;

		Symbol temp = Symbol(name, type, staLine, value);
		this->list.insert(list.begin()+res_bise.pos,temp);
		return res_bise.pos;
	}
	//return -1 if name is not in table, return position if name exists in table.
	int lookup(string name)
	{
		ResBise res_bise = bise_symTab(name);
		if (res_bise.exist)return res_bise.pos;
		else return -1;
	}

	void output()
	{
		for (vector<Symbol>::iterator it = list.begin(); it != list.end(); it++)
			it->output();
	}

	void getAssemData(ofstream& fout)
	{
		int offset=0;
		fout << ".386" << endl;
		fout << ".Model Flat, StdCall" << endl;
		fout << "Option Casemap :None" << endl;
		fout << "Include \\masm32\\include\\msvcrt.inc" << endl;
		fout << "Include \\masm32\\include\\masm32rt.inc" << endl;
		fout << "IncludeLib \\masm32\\lib\\msvcrt.lib" << endl;
		fout << ".DATA" << endl;
		for (vector<Symbol>::iterator it = this->list.begin(); it != list.end(); it++){
			switch (it->type)
			{
			case CONSCHAR:
			case CONSINT:fout << "CONGLOBAL" << it->name << " EQU " << it->value << 'd' << endl; break;
			case VARCHAR:fout << "VAR" << it->name<<" DD ?"<<endl; it->offset = offset; offset += 1; break;
			case VARINT:fout << "VAR" << it->name<<" DD ?"<<endl; it->offset = offset; offset += 4; break;
			case CHARRAY:fout << "VAR" << it->name << " DD " << it->value << " DUP(0)" << endl; it->offset = offset; offset += (it->value); break;
			case INTARRAY:fout << "VAR" << it->name << " DD " << it->value << " DUP(0)" << endl; it->offset = offset; offset += (4 * it->value); break;
			}
		}
		//fout << "DATA ENDS" << endl;
	}
private:
	/****Function for Grammatical Analysis****/
	////===>return -1 if input exists in symTab,return i+1 which symTab[i].name<input but symTab[i+1].name>input.
	ResBise bise_symTab(string name)
	{
		int beg = 0, sto = this->list.size() - 1;
		int mid;
		while (beg <= sto){
			mid = (beg + sto) / 2;
			if (name<this->list[mid].name)
				sto = mid - 1;
			else if (name>this->list[mid].name)
				beg = mid + 1;
			else return { true, mid };
		}
		//if (name == reserve_word[beg])return -1;
		return { false, beg };
	}
	//string dtoh(int n)
	//{
	//	string tmp;
	//	string res;
	//	if (n == 0)return "0H";
	//	while (n > 0){
	//		tmp
	//	}
	//}
};

//CopyRight by Librian
