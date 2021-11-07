#include <iostream>
#include <fstream>
#include "lexical.h"
#include "grammar.h"
#include "ir/ir.h"

using namespace std;

ifstream fin("testfile.txt");
ofstream fout("output.txt");
ofstream eout("error.txt");
ofstream iout("ir.txt");

Output output;
string input;
IR IR_1;

void read() {
    string s;
    while(getline(fin, s)) {
        s += "\n";
        input += s;
    }
}

void print(string outString) {
    //cout << outString << endl;
    //fout << outString << endl;
    //eout << outString << endl;
    iout << outString << endl;
}

int main() {
    read();
    //Lexical lexical = Lexical(input);
    Grammar grammar = Grammar(input);
    Node* root = grammar.getRoot();
    root -> traversal();
    print(IR_1.toString());
    //print(output.to_string());
    //print(lexical.to_string());
    return 0;
}
