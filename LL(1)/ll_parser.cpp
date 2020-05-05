#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <utility>
using namespace std;

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
class CFG{
    class symbol{
    public:
        string sym;
        int tokenID;
        set<int> first;
        set<int> follow;
        bool isTerminal;
        int nullProduction;
        vector<int> productions;

    public:
        symbol(string s, int i, bool t):sym(s),tokenID(i),isTerminal(t),nullProduction(-1){}
    };
    vector<symbol> table;
    vector<pair<int,vector<int>>> productions;
    map<string,int> symbol_map;
    map<pair<int, int>, int> parsingTable;

    int start_symbol;
public: 
    CFG(){
        table.push_back(symbol("NULL",0,false));
        symbol_map["NULL"]=0;
        table.push_back(symbol("$",1,true));
        symbol_map["$"]=1;
    }
    void cerateCFG(fstream &);
    bool parseGrammer();
    void printParsingTable();
    void parseString(string S);
    void print();
    int count_firsts();
    int count_follows();
    void compute_firsts();
    void compute_follows();
    void printSet(set<int> &);
    void printProduction(int );
    bool insertIntoParsingTable(int, int , int);
    set<int> get_first(int , int );
    string getProductionString(int);
};

int main(){
    fstream IN_File, OUT_File;
    string filename="input2";
    cout << "Enter filename - ";
    cin >> filename;
    IN_File.open(filename, ios::in);
    if(IN_File.is_open()){
        CFG T;
        T.cerateCFG(IN_File);
        bool tmp = T.parseGrammer();
        if(tmp)
            T.print();
        
    }else{
        cout << "file " << filename << " doesn't exist.\n";
    }
    cout << endl << endl << "Submitted by -" << endl;
    cout << "\t Mudit Singh Bisht" << endl;
    cout << "\t      17BCS013" << endl;
    cout << "\t  Computer Engineering (VI Sem)" << endl
         << endl;

    IN_File.close();
    return 0;
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

void CFG::cerateCFG(fstream &file){
    char c;
    bool t;
    int n_productions,n;
    string s;

    while(!file.eof()){
        getline(file, s);
        this->parseString(s);
    }
    this->start_symbol = this->productions[0].first;
    n = this->table.size();
    for (int i = 2;i<n;i++)
        if(!this->table[i].isTerminal)
            for (int j = 1; j < n;j++)
                if(this->table[j].isTerminal)
                    this->parsingTable[make_pair(i, j)] = -1;
}

void CFG::print(){
    cout << endl << "Start symbol - " << this->table[this->start_symbol].sym << endl;
    int N = this->table.size();
    for(int i=2;i<N;i++){
        symbol &p = this->table[i];
        if(!p.isTerminal){
            cout << "\nFirst(" << p.sym << ") = ";
            printSet(p.first);
            cout << "\nFollow(" << p.sym <<") = ";
            printSet(p.follow);
            cout << endl;
            
        }
    }
    this->printParsingTable();
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
void CFG::printSet(set<int>& s){
    cout << "{";
    set<int>::iterator it = s.begin();
    if(it!=s.end()){
        switch(this->table[*it].sym[0]){
            case '{':
            case '}':
            case ',':
                cout << "'" << this->table[*it].sym << "'";
                break;
            default:
                cout << this->table[*it].sym;
            }
        it++;
    }
    while (it != s.end()){
        switch(this->table[*it].sym[0]){
            case '{':
            case '}':
            case ',':
                cout << ", '" << this->table[*it].sym << "'";
                break;
            default:
                cout << ", " <<this->table[*it].sym;
            }
        it++;
    }
    cout << "}";
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

bool CFG::parseGrammer(){
    this->compute_firsts();
    this->compute_follows();

    int N_productions = this->productions.size();

    for (int I = 0; I < N_productions;I++){
        int left = this->productions[I].first;
        set<int> first = this->get_first(I, 0);
        // set<int> first = this->table[left].first;
        set<int>::iterator it = first.begin();
        
        for (; it != first.end();it++){
            if(*it==0){
                int t = this->table[left].nullProduction;
                set<int> &follow = this->table[left].follow;
                set<int>::iterator it = follow.begin();
                for (; it != follow.end();it++)
                    if(!this->insertIntoParsingTable(left, *it, t))
                        return false;
            }else{
                if(!this->insertIntoParsingTable(left, *it, I))
                    return false;
            }
        }
    }

    return true;
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
bool CFG::insertIntoParsingTable(int nt, int t, int val){
    if(this->parsingTable[make_pair(nt,t)]==-1 || this->parsingTable[make_pair(nt,t)]==val){
        this->parsingTable[make_pair(nt,t)] = val;
        return true;
    }else{
        cout << "Double entries on " << this->table[nt].sym << "  " << this->table[t].sym << " - \n";
        this->printProduction(this->parsingTable[make_pair(nt,t)]);
        this->printProduction(val);
        return false;
    }
}

void CFG::printParsingTable(){
    cout << endl << endl << "Parsing Table -> \n";
    int N = this->table.size();
    cout << setWidth(10, "") << "|";
    for (int i = 0; i < N;i++)
        if(this->table[i].isTerminal)
            cout << setWidth(15,this->table[i].sym) << "|";
    cout << endl;
    cout << setWidth(10,"________") << "|";
    for (int i = 0; i < N;i++)
        if(this->table[i].isTerminal)
            cout << setWidth(15,"___________") << "|";
    cout << endl;
    
    for (int i = 2;i<N;i++){
        if(!this->table[i].isTerminal){
            cout << setWidth(10, this->table[i].sym) << "|";
            for (int j = 0;j<N;j++){
                if(this->table[j].isTerminal){
                    if(this->parsingTable[make_pair(i,j)]!=-1)
                        cout << setWidth(15, this->getProductionString(this->parsingTable[make_pair(i, j)])) << "|";
                    else
                        cout << setWidth(15, "-----") << "|";
                }
            }
            cout << endl;
        }
    }
}