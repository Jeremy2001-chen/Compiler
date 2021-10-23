#include <iostream>
#include <fstream>
#include "lexical.h"
#include "grammar.h"

using namespace std;

ifstream fin("testfile.txt");
ofstream fout("output.txt");
ofstream eout("error.txt");

Output output;
string input;

void read() {
    string s;
    while(getline(fin, s)) {
        s += "\n";
        input += s;
    }
}

void print(string outString) {
    cout << outString << endl;
    //fout << outString << endl;
    eout << outString << endl;
}

int main() {
    read();
    //Lexical lexical = Lexical(input);
    Grammar grammar = Grammar(input);
    print(output.to_string());
    //print(lexical.to_string());
    return 0;
}
