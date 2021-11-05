//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_SINGLE_EXP_H
#define COMPILER_SINGLE_EXP_H

#include "node.h"
#include <cassert>
#include <utility>
#include "value.h"

#define lch ch[0]
class SingleExp: public Node {
protected:
    Node* ch[1] = {nullptr};
    string sign;
public:
    SingleExp() {
        classType = SingleExpType;
        lch = nullptr;
    }
    string getSign() const {
        return sign;
    }
    void setLch(Node* Lch) {
        if (lch)
            size -= lch->getSize();
        lch = Lch;
        if (lch)
            size += lch->getSize();
    }
    void check() override {
        cout << "SingleExp check correct!" << endl;

    }
    void traversal() override {

    }
    virtual int op(int) = 0;
    Node* optimize() override {
        if (!lch -> getConstType())
            return this;
        ConstValue* lchConst = (ConstValue*)lch;
        Number* number = new Number(op(lchConst->getValue()));
        return number;
    }
};

class UnaryExp: public SingleExp {
public:
    explicit UnaryExp(string _sign) {
        sign = std::move(_sign);
        classType = UnaryExpType;
    }
    void check() override {
        assert(sign == "+" || sign == "-" || sign == "!");
        cout << "UnaryExp check correct!" << endl;
        //lch->check();
    }
    void traversal() override {
        cout << sign << endl;
        lch->traversal();
    }
    int op(int l) override {
        switch (sign[0]) {
            case '+': return l;
            case '-': return -l;
            case '!': return !l;
            default: exit(-2);
        }
    }
};

#endif //COMPILER_SINGLE_EXP_H
