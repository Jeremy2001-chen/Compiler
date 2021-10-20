//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_BLOCK_H
#define COMPILER_BLOCK_H

#include "node.h"
#include <vector>
#include <cstring>
#include "../error.h"
#include "output.h"
using namespace std;

extern Output output;

class Block: public Node {
private:
    vector<Node> blockItem;
public:
    Block() = default;
    void addBlockItem(Node node) {
        blockItem.push_back(node);
    }
    void check() override {
        assert(blockItem);
        for (auto blockIt: blockItem) {
            blockIt.check();
        }
        cout << "Block check correct!" << endl;
    }
    void traversal() override {
        for (auto blockIt: blockItem) {
            blockIt.traversal();
        }
    }
};

class IfStmt: public Node {
private:
    vector<Node> cond;
    vector<Node> tran;
public:
    IfStmt() = default;
    void addTran(Node _cond, Node _tran) {
        cond.push_back(_cond);
        tran.push_back(_tran);
    }
    void addElseTran(Node _tran) {
        tran.push_back(_tran);
    }
    void check() override {

    }
    void traversal() override {

    }
};

class WhileStmt: public Node {
private:
    Node cond;
    Block block;
public:
    WhileStmt(Node _cond, Block _block) {
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
    Node returnExp;
public:
    ReturnStmt() {
        returnExp = nullptr;
        type = -1;
    }
    ReturnStmt(Node exp) {
        returnExp = exp;
        type = exp.getType();
    }
    void check() override {

    }
    void traversal() override {

    }
};

class PrintfStmt: public Node {
private:
    vector<string> form;
    vector<Node> exp;
public:
    PrintfStmt(string format, vector<Node> _exp, int _line) {
        string tmp = "";
        bool errorChar = false;
        int len = format.size();
        for (int i = 0; i < len; i++) {
            if (c[i] != 32 && c[i] != 33 && (c[i] < 40 || c[i] > 126))
                errorChar = true;
            if (c[i] == '\\') {
                if (!(i < len - 1 && c[i+1] == '\n'))
                    errorChar = true;
            }
            if (i < len - 1 && c[i] == '%' && c[i+1] == 'd') {
                form.push_back(tmp);
                tmp = "";
            } else {
                tmp += c[i];
            }
        }
        form.push_back(tmp);
        exp = std::move(_exp);
        this.line = _line;
        if (errorChar) {
            output.addError(PrintParameterNumError(_line, format.size(), exp.size()));
        }
    }
    void check() override {

    }
    void traversal() override {

    }
};
#endif //COMPILER_BLOCK_H
