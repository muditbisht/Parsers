#include "slr.hpp"
using namespace std;

int main(){
    fstream IN_File, OUT_File;
    string filename="input2";
    cout << "Enter filename - ";
    cin >> filename;
    IN_File.open(filename, ios::in);
    if(IN_File.is_open()){
        CFG C;
        SLRTable T;
        C.cerateCFG(IN_File);
        bool tmp = create_SLRTable(C, T);
        if(tmp){
            C.print();
            T.print(C);
        }
        
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