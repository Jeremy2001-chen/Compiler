//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef COMPILER_BINARY_EXP_H
#define COMPILER_BINARY_EXP_H

#define lch ch[0]
#define rch ch[1]

#include "node.h"
#include <cassert>

class BinaryExp: public Node{
protected:
    Node ch[2];
    string sign;
public:
    string getSign() const {
        return sign;
    }
    void setLch(const Node& Lch) {
        lch = Lch;
    }
    void setRch(const Node& Rch) {
        rch = Rch;
    }
};

class MulExp: public BinaryExp{
public:
    MulExp(string _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "*");
        cout << "MulExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << "*" << endl;
        lch.traversal();
        rch.traversal();
    }
};

class AddExp: public BinaryExp{
public:
    AddExp(char _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "+" || sign == "-");
        cout << "AddExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << sign << endl;
        lch.traversal();
        rch.traversal();
    }
};

class RelExp: public BinaryExp{
public:
    RelExp(string _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "<" || sign == ">" || sign == ">=" || sign == "<=");
        cout << "RelExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << sign << endl;
        lch.traversal();
        rch.traversal();
    }
};

class EqExp: public BinaryExp{
public:
    EqExp(string _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "==" || sign == "!=");
        cout << "EqExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << sign << endl;
        lch.traversal();
        rch.traversal();
    }
};

class LAndExp: public BinaryExp{
public:
    LAndExp(string _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "&&");
        cout << "LAndExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << sign << endl;
        lch.traversal();
        rch.traversal();
    }
};

class LOrExp: public BinaryExp{
public:
    LOrExp(string _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "||");
        cout << "LOrExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << sign << endl;
        lch.traversal();
        rch.traversal();
    }
};

class AssignExp: public BinaryExp{
public:
    AssignExp() {
        sign = ":=";
    }
    void check() override {
        assert(sign == ":=");
        cout << "AssignExp check correct!" << endl;
        lch.check();
        rch.check();
    }
    void traversal() override {
        cout << sign << endl;
        lch.traversal();
        rch.traversal();
    }
};
#endif //COMPILER_BINARY_EXP_H
