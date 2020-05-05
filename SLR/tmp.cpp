#include <bits/stdc++.h>
using namespace std;

bool comp(set<pair<int,int>> &A,  set<pair<int,int>> &B){
    set<pair<int, int>>::iterator it = A.begin();
    
    if(A.size()!=B.size())
        return false;
    for (; it != A.end();it++)
        if(A.count(*it)==0)
            return false;
    return true;
}

int main(){
    set<pair<int, int>> P,T;
    P.insert({0, 1});
    P.insert({1, 3});

    set<pair<int, int>> tmp;

    while(1){
        int a,b;
        tmp.clear();
        cin >> a >> b;
        while(a+b){
            tmp.insert({a, b});
            cin >> a >> b;
        }
        T = tmp;
        if(P==T){
            cout << "Same" << endl;
        }else
            cout << "NOT Same" << endl;
    
    }
    return 0;
}