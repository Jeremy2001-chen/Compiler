#include <iostream>
#include <fstream>
#include "lexical.h"
#include "grammar.h"
#include "ir/ir.h"
#include "mips/mips.h"
#include "mips/mips_table.h"
#include "ir/ir_code.h"

using namespace std;

ifstream fin("testfile.txt");
ofstream fout("output.txt");
ofstream eout("error.txt");
ofstream iout("ir.txt");
ofstream mout("mips.txt");

Output output;
string input;
IR IR_1;
Mips *mips; MipsTable* mipsTable; MipsOutput* mipsOutput;

void read() {
    string s;
    while(getline(fin, s)) {
        s += "\n";
        input += s;
    }
}

void print(string outString, ofstream &ofs) {
    //cout << outString << endl;
    //fout << outString << endl;
    //eout << outString << endl;
    //iout << outString << endl;
    ofs << outString << endl;
}

int main() {
    read();
    //Lexical lexical = Lexical(input);
    Grammar grammar = Grammar(input);
    Node* root = grammar.getRoot();
    root -> traversal();
    print(IR_1.toString(), iout);
    mipsTable = new MipsTable();
    mipsOutput = new MipsOutput();
    mips = new Mips(IR_1, mipsTable, mipsOutput);
    //print(output.to_string());
    //print(lexical.to_string());
    print(mipsOutput -> toString(), mout);
    return 0;
}
