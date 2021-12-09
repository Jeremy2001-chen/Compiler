#include <iostream>
#include <fstream>
#include "lexical.h"
#include "grammar.h"
#include "mips/mips.h"
#include "ir/ir_new.h"
#include "mips/register.h"

using namespace std;

ifstream fin("testfile.txt");
ofstream fout("output.txt");
ofstream eout("error.txt");
ofstream iout("ir.txt");
ofstream mout("mips.txt");
ofstream nout("ir_new.txt");
ofstream kout("ir_new1.txt");
ofstream gout("ir_new2.txt");



Output output;
string input;
IR IR_1, IR_2;
Mips *mips; MipsTable* mipsTable; MipsOutput* mipsOutput;
Register* aRegister;

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
    aRegister = new Register();
    IrNew *irNew = new IrNew(&IR_1);
    print(irNew -> toString(), nout);
    removeAddZero(irNew);
    print(irNew -> toString(), kout);
    constSpread(irNew);
    print(irNew -> toString(), gout);
    irNew -> toMips();
    //    vector<IrCode*>* temp = irNew -> toIR();
//    for (auto code: *temp)
//        IR_2.add(code);
//    IR_2.setGlobalDeclEnd(IR_1.getGlobalDeclEnd());
//    //print(irNew -> toString(), nout);
//    print(IR_2.toString(), nout);
    //mips = new Mips(IR_2, mipsTable, mipsOutput);
    //print(output.to_string());
    //print(lexical.to_string());*/
    print(mipsOutput -> toString(), mout);
    return 0;
}
