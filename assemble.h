#include<string>
#define STACKDEP	1024
#define TMPVAR 1
#define LOCVAR 2
#define GLOVAR 3
#define PARVAR 4
#define RETVAR 5

using namespace std;
class varSrc{
public:
	int src;
	int index;
};
class tmpVarPos{
public:
	string name;
	int offset;//
	tmpVarPos(string name) :name(name)
	{
		offset = -1;
	}
	void setOffset(int os)
	{
		offset = os;
	}
};
class tmpVarList{
public:
	vector<tmpVarPos> list;
public:
	void insert(string tmp)
	{
		int beg = 0, sto = this->list.size() - 1;
		int mid;
		while (beg <= sto){
			mid = (beg + sto) / 2;
			if (tmp<this->list[mid].name)
				sto = mid - 1;
			else if (tmp>this->list[mid].name)
				beg = mid + 1;
			else return;
		}
		//if (name == reserve_word[beg])return -1;
		list.insert(list.begin()+beg,tmpVarPos(tmp));
	}
	void clear(){
		list.clear();
	}
};

//CopyRight by Librian