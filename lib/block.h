//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_BLOCK_H
#define COMPILER_BLOCK_H

#include "node.h"
#include <utility>
#include <vector>
#include <cstring>
#include "../error.h"
#include "../output.h"
using namespace std;

extern Output output;

class Block: public Node {
private:
    vector<Node*> blockItem;
public:
    Block() = default;
    void addBlockItem(Node* node) {
        blockItem.push_back(node);
    }
    void check() override {
        //assert(blockItem);
        /*for (auto &blockIt: blockItem) {
            blockIt.check();
        }
        cout << "Block check correct!" << endl;*/
    }
    void traversal() override {
        /*for (auto &blockIt: blockItem) {
            blockIt.traversal();
        }*/
    }
};

class IfStmt: public Node {
private:
    vector<Node*> cond;
    vector<Node*> tran;
public:
    IfStmt() = default;
    void addTran(Node* _cond, Node* _tran) {
        cond.push_back(_cond);
        tran.push_back(_tran);
    }
    void addElseTran(Node* _tran) {
        tran.push_back(_tran);
    }
    void check() override {

    }
    void traversal() override {

    }
};

class WhileStmt: public Node {
private:
    Node* cond;
    Block* block;
public:
    WhileStmt(Node* _cond, Block* _block) {
        cond = _cond;
        block = _block;
    }
    void check() override {

    }
    void traversal() override {

    }
};

class BreakStmt: public Node {
public:
    BreakStmt() = default;
    void check() override {

    }
    void traversal() override {

    }
};

class ContinueStmt: public Node {
public:
    ContinueStmt() = default;
    void check() override {

    }
    void traversal() override {

    }
};

class ReturnStmt: public Node {
private:
    Node* returnExp;
public:
    ReturnStmt() {
        returnExp = nullptr;
        type = -1;
    }
    ReturnStmt(Node* exp) {
        returnExp = exp;
        type = exp->getType();
    }
    void check() override {

    }
    void traversal() override {

    }
};

class PrintfStmt: public Node {
private:
    string format;
    vector<string> form;
    vector<Node*> exp;
public:
    PrintfStmt(string _format, vector<Node*> _exp,  int _formatLine, int _printfLine) {
        format = std::move(_format);
        exp = std::move(_exp);
        line = _printfLine;
        checkFormatError(_formatLine);
        int formatSize = form.size(), expSize = exp.size();
        if (formatSize != expSize + 1) {
            output.addError(PrintParameterNumError(_printfLine, formatSize - 1, expSize));
        }
    }

    void checkFormatError(int _formatLine) {
        string tmp = "";
        bool errorChar = false;
        int len = format.size();
        for (int i = 0; i < len; i++) {
            if (format[i] != 32 && format[i] != 33 && (format[i] < 40 || format[i] > 126))
                errorChar = true;
            if (format[i] == '\\') {
                if (!(i < len - 1 && format[i+1] == '\n'))
                    errorChar = true;
            }
            if (i < len - 1 && format[i] == '%' && format[i+1] == 'd') {
                form.push_back(tmp);
                tmp = "";
            } else {
                tmp += format[i];
            }
        }
        form.push_back(tmp);
        if (errorChar) {
            output.addError(IllegalCharacterError(_formatLine));
        }
    }

    void check() override {

    }
    void traversal() override {

    }
};
#endif //COMPILER_BLOCK_H
