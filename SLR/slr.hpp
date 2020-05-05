#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <utility>
using namespace std;
string setWidth(int, string );
void error(string);
vector<string> split(string);
class CFG;
class SLRTable;
class symbol;
class Item;

class Item{
    public:
        set<pair<int, int>> productions;
        map<int, int> Goto;
};


class symbol{
    public:
        string sym;
        int tokenID;
        set<int> follow;
        set<int> first;
        bool isTerminal;
        int nullProduction;
        vector<int> productions;

        symbol(string s, int i, bool t):sym(s),tokenID(i),isTerminal(t),nullProduction(-1){}
};
class SLRTable{
public:
    vector<Item> items;
    vector<map<int,pair<char, int>>> ACTION;
    vector<map<int, int>> GOTO;
    int Goto(CFG &C, int item, int symbol);
    friend bool create_SLRTable(CFG &, SLRTable &);
    void print(CFG &);
};

class CFG{
public:
    vector<symbol> table;
    vector<pair<int,vector<int>>> productions;
    map<string,int> symbol_map;
    int start_symbol;
 
    CFG(){
        table.push_back(symbol("NULL",0,false));
        symbol_map["NULL"]=0;
        table.push_back(symbol("$",1,true));
        symbol_map["$"]=1;
    }
    void cerateCFG(fstream &);
    void parseString(string S);
    int count_follows();
    int count_firsts();
    void compute_firsts();
    void compute_follows();
    void print();
    void printProduction(int );
    set<int> get_first(int , int );
    string getProductionString(int);
    friend bool create_SLRTable(CFG &, SLRTable &);
    void closure(set<pair<int, int>>&);
};


string setWidth(int width, string S){
    if(width<=S.length())
        return S;
    width -= S.length();
    return string(width / 2, ' ') + S + string(width - width / 2, ' ');

}
void error(string S){
    cout << "[ERROR]  " << S << endl;
    exit(1);
}
vector<string> split(string S){
    vector<string> tmp;
    int i = 0, j = 0;
    int N = S.length();
    for (; i < N;){
        while(i<N && S[i]==' ')
            i++;
        j=i;
        while(i<N && S[i]!=' ')
            i++;
        if(i>j)
            tmp.push_back(S.substr(j, i - j));
    }
    return tmp;
}


void CFG::cerateCFG(fstream &file){
    char c;
    bool t;
    int n_productions;
    string s;

    this->table.push_back(symbol("",0,0));
    this->productions.push_back({2, vector<int>{3}});
    while(!file.eof()){
        getline(file, s);
        this->parseString(s);
    }
    s = this->table[this->productions[1].first].sym;
    this->symbol_map[s+"`"] = 2;
    this->table[2].sym = s+"`";
    this->productions[0] = make_pair(this->symbol_map[s + "`"], vector<int>{this->symbol_map[s]});
    this->start_symbol = this->symbol_map[s+"`"];
}


int CFG::count_firsts(){
    int count=0;
    for(auto p:this->table){
        count+=p.first.size();
    }
    return count;
}
int CFG::count_follows(){
    int count=0;
    for(auto p:this->table){
        count+=p.follow.size();
    }
    return count;
}

void CFG::compute_firsts(){
    vector<symbol>::iterator symb_it = this->table.begin();
    vector<pair<int,vector<int>>>::iterator prod_it = this->productions.begin();

    for(;symb_it!=this->table.end();symb_it++)
        if(symb_it->isTerminal)
            symb_it->first.insert(symb_it->tokenID);

    for(;prod_it!=this->productions.end();prod_it++)
        if(prod_it->second[0]==0){
            this->table[prod_it->first].first.insert(0);
            this->table[prod_it->first].nullProduction = prod_it - this->productions.begin();
        }
            

    int old_count=0,count=0;
    int N_productions = this->productions.size();
    do{
        for(int I=0;I<N_productions;I++){
            int non_terminal = this->productions[I].first;
            set<int> tmp = this->get_first(I, 0);
            set<int>::iterator it = tmp.begin();
            for (; it!=tmp.end();it++)
                this->table[non_terminal].first.insert(*it);

            if(tmp.count(0)){
                if(this->table[non_terminal].nullProduction==-1){
                    this->productions.push_back(make_pair(non_terminal, vector<int> {0}));
                    this->table[non_terminal].nullProduction = this->productions.size() - 1;
                    this->table[non_terminal].productions.push_back(this->productions.size() - 1);
                }else if(this->table[non_terminal].nullProduction==I)
                    this->table[non_terminal].nullProduction = I;
                else{
                    cout << this->table[non_terminal].sym << " gives null production from different productions." << endl;
                    this->printProduction(this->table[non_terminal].nullProduction);
                    this->printProduction(I);
                    error("It's not LL(1) grammer.");
                }
            }
        }
        old_count = count;
        count = this->count_firsts();
    }while(old_count!=count);
}

void CFG::compute_follows(){
    this->table[this->start_symbol].follow.insert(this->symbol_map["$"]);
    int count=0,old_count=0;
    do{
        int N_productions = this->productions.size();
        for(int I=0;I<N_productions;I++){
            bool cont=true;
            int non_terminal = this->productions[I].first;
            int n = this->productions[I].second.size();
            for(int i=n-1;i>=0;i--){
                if(cont){
                    int tmp = this->productions[I].second[i];
                    set<int>::iterator set_it = this->table[non_terminal].follow.begin();
                    for(;set_it!=this->table[non_terminal].follow.end();set_it++)
                        this->table[tmp].follow.insert(*set_it);
                }
                if(i<(n-1)){
                    int tmp1 = this->productions[I].second[i];
                    set<int> tmp = this->get_first(I, i + 1);
                    set<int>::iterator set_it = tmp.begin();
                    for(;set_it!=tmp.end();set_it++)
                        if(*set_it)
                            this->table[tmp1].follow.insert(*set_it);
                }
                cont = cont && (this->table[this->productions[I].second[i]].first.count(0)>0);
            }
        }
        old_count=count;
        count=this->count_follows();
    }while(old_count!=count);
}

string CFG::getProductionString(int i){
    string tmp;
    tmp =  this->table[this->productions[i].first].sym + " -> ";
    vector<int>::iterator it = this->productions[i].second.begin();
    vector<int>::iterator end = this->productions[i].second.end();
    for (; it != end;it++){
        tmp += this->table[*it].sym;
    }
    return tmp;
}
void CFG::printProduction(int i){
    cout << this->table[this->productions[i].first].sym << " -> ";
    vector<int>::iterator it = this->productions[i].second.begin();
    vector<int>::iterator end = this->productions[i].second.end();
    for (; it != end;it++){
        cout << this->table[*it].sym << " ";
    }
    cout << endl;
}
void CFG::parseString(string S){

    string left,right;
    vector<string> tmp = split(S);
    int i = 0,j;
    int N = tmp.size();

    if(N==0)
        return;
    
    if(N<3)
        error("production should like  <non-terminal> -> E1 E2 E3");
    if (tmp[1] != "->")
        error("Invalid input");    
    if(tmp[2]=="NULL" && N!=3)
        error("NULL transition should not have other non terminals");
    for (int i = 3; i < N;i++)
        if(tmp[i]=="NULL")
            error("NULL transition should not have other non terminals");

    left = tmp[0];

    vector<int> V;
    if(this->symbol_map.find(left)==this->symbol_map.end()){
        int n = this->table.size();
        this->symbol_map[left] = n;
        this->table.push_back(symbol(left, n, false));
    }else{
        this->table[this->symbol_map[left]].isTerminal = false;
    }
    for (i = 2; i < N;i++){
        if(this->symbol_map.find(tmp[i])==this->symbol_map.end()){
            int n = this->table.size();
            this->symbol_map[tmp[i]] = n;
            this->table.push_back(symbol(tmp[i], n, true));
        }
        V.push_back(this->symbol_map[tmp[i]]);
    }
    this->table[this->symbol_map[left]].productions.push_back(this->productions.size());
    this->productions.push_back(make_pair(this->symbol_map[left], V));
}
void CFG::print(){
    cout << endl << "Start symbol - " << this->table[this->start_symbol].sym << endl;
    int N = this->productions.size();
    for (int i = 0; i < N;i++)
        this->printProduction(i);
}
set<int> CFG::get_first(int production, int index ){
    if(index==0 && this->productions[production].second[index]==0)
        return set<int>{0};
    set<int> firsts;
    bool nxt = true;
    int N = this->productions[production].second.size();
    for (; index < N&&nxt;index++){
        set<int> &tmp = this->table[this->productions[production].second[index]].first;
        set<int>::iterator it = tmp.begin();
        for (; it != tmp.end();it++)
            if(*it)
                firsts.insert(*it);
        nxt = tmp.count(0);
    }
    if(index==N && nxt)
        firsts.insert(0);
    return firsts;
}

int SLRTable::Goto(CFG &C, int item, int symbol){
    set<pair<int,int>> P;
    set<pair<int, int>>::iterator it = this->items[item].productions.begin();
    for (;it!=this->items[item].productions.end();it++){
        if(C.productions[it->first].second.size()>it->second && C.productions[it->first].second[it->second]==symbol)
            P.insert({it->first, it->second + 1});
    }

    if(P.size()==0)
        return -1;
    C.closure(P);
    
    for (int i = 0; i < this->items.size();i++)
        if(this->items[i].productions==P)
            return i;
    
    this->items.push_back(Item());
    int N = this->items.size() - 1;
    this->items[N].productions = P;
    return N;
}
void CFG::closure(set<pair<int,int>>& S){
    set<pair<int, int>>::iterator it;
    int count=S.size(), old_count = 0;
    CFG &C = *this;
    do{
        it = S.begin();

        for (;it!=S.end();it++){
            if(C.productions[it->first].second[it->second]!=0 && C.productions[it->first].second.size()>it->second  && !C.table[C.productions[it->first].second[it->second]].isTerminal){
                int non_terminal = C.productions[it->first].second[it->second];
                for (int i = 0; i < C.table[non_terminal].productions.size();i++){
                    S.insert({C.table[non_terminal].productions[i], 0});
                }
            }
        }

        old_count = count;
        count = S.size();
    } while (old_count != count);

}

bool create_SLRTable(CFG &C, SLRTable &S){
    C.compute_firsts();
    C.compute_follows();

    S.items.push_back(Item());
    S.items[0].productions.insert({0, 0});
    C.closure(S.items[0].productions);

    for (int i = 0; i < S.items.size();i++)
        for (int j = 3; j < C.table.size();j++)
            S.items[i].Goto[j] = S.Goto(C, i, j);


    int N = S.items.size();
    S.GOTO.resize(N);
    S.ACTION.resize(N);
    for (int i = 0; i < N;i++)
        for (int j = 0; j < C.table.size();j++){
            if(C.table[j].isTerminal)
                S.ACTION[i][j] = make_pair('E', -1);
            else
                S.GOTO[i][j] = -1;
        }
    
    for(int i=0;i<N;i++){
        // if(S.items[i].productions.count(make_pair(0,1))==1)
            // S.ACTION[i][C.symbol_map["$"]] = make_pair('A', 0);
    }

    for (int i = 0; i <N;i++){
        for (int j = 3; j < C.table.size();j++){
            if(S.items[i].Goto[j]!=-1){
                if(C.table[j].isTerminal){
                        S.ACTION[i][j] = make_pair('S', S.items[i].Goto[j]);
                }else{
                    S.GOTO[i][j] = S.items[i].Goto[j];
                }
            }
        }
    }
    
    for (int i = 0;i<N;i++){
        int m = S.items[i].productions.size();
        set<pair<int, int>>::iterator sit = S.items[i].productions.begin();
        for (; sit != S.items[i].productions.end();sit++){
            if(C.productions[sit->first].second.size()==sit->second){
                set<int>::iterator it = C.table[C.productions[sit->first].first].follow.begin();
                for (;it!=C.table[C.productions[sit->first].first].follow.end();it++){
                    if(S.ACTION[i][*it]==make_pair('E',-1))
                        S.ACTION[i][*it] = make_pair('R', sit->first);
                    else{
                        cout << "Given grammer gives Shift/Reduce conflict - "<< S.ACTION[i][*it].first<<S.ACTION[i][*it].second << "/R" <<  sit->first << endl;
                        error("Not an SLR grammer.");
                    }
                }
            }
        }
    }

    return true;
}

void SLRTable::print(CFG &C){
    SLRTable &S = *this;
    int N = S.items.size();
    int M = C.table.size();

    cout << endl
          << endl;
    printf("|state|");
    for (int j = 0;j<M;j++)
        if(C.table[j].isTerminal)
            cout << setWidth(5, C.table[j].sym) << "|";
    for (int j = 3;j<M;j++)
        if(!C.table[j].isTerminal)
            cout << setWidth(4, C.table[j].sym) << "|";
    cout << endl;
    printf("|_____|");
    for (int j = 0;j<M;j++)
        if(C.table[j].isTerminal)
            cout << setWidth(5, "___") << "|";
    for (int j = 3;j<M;j++)
        if(!C.table[j].isTerminal)
            cout << setWidth(4, "__") << "|";
    cout << endl;

    for (int i = 0; i < N;i++){
        printf("|  %02d |", i);
        for (int j = 0;j<M;j++){
            if(C.table[j].isTerminal){
                if(S.ACTION[i][j] == make_pair('R',0))
                    printf(" %3s |", "ACC");
                else if(S.ACTION[i][j].first!='E')
                    printf(" %c%02d |", S.ACTION[i][j].first, S.ACTION[i][j].second);
                else
                    printf(" %3s |", "-");
            }
        }
        for (int j = 3;j<M;j++){
            if(!C.table[j].isTerminal){
                if(S.GOTO[i][j]!=-1)
                    printf(" %02d |", S.GOTO[i][j]);
                else
                    printf(" %2s |", " ");
            }
        }
        cout << endl;
    }
}