//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef COMPILER_BINARY_EXP_H
#define COMPILER_BINARY_EXP_H

#define lch ch[0]
#define rch ch[1]

#include "node.h"
#include <cassert>
#include <utility>
#include "../type.h"

extern IR IR_1;
extern IrTableList irTableList_1;
class BinaryExp: public Node{
protected:
    Node* ch[2] = {nullptr, nullptr};
    string sign;
public:
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
    void setRch(Node* Rch) {
        if (rch)
            size -= rch->getSize();
        rch = Rch;
        if (rch)
            size += rch->getSize();
    }
    virtual int op(int, int) = 0;
    Node* optimize() override {
        if (!lch -> getConstType() || !rch -> getConstType())
            return this;
        ConstValue* lchConst = (ConstValue*)lch, *rchConst = (ConstValue*)rch;
        Number* number = new Number(op(lchConst->getValue(), rchConst->getValue()));
        return number;
    }
    void traversal() override {
        string target, left, right;
        if (lch->getClassType() == VariableType && lch->getSize() == 1)
            left = irTableList_1.getIrName(((Variable*)lch)->getName());
        else {
            lch -> traversal();
            left = irTableList_1.getTopTemIrName();
        }
        if (rch->getClassType() == VariableType && rch->getSize() == 1)
            right =irTableList_1.getIrName(((Variable*)rch)->getName());
        else {
            rch -> traversal();
            right = irTableList_1.getTopTemIrName();
        }
        target = irTableList_1.allocTem();
        IR_1.add(new IrBinaryOp(target, left, sign, right));
    }
};

class MulExp: public BinaryExp{
public:
    explicit MulExp(string _sign) {
        sign = std::move(_sign);
        classType = MulType;
    }
    void check() override {
        assert(sign == "*" || sign == "/" || sign == "%");
        cout << "MulExp check correct!" << endl;
        /*lch.check();
        rch.check();*/
    }
    int op(int l, int r) override {
        switch (sign[0]) {
            case '*': return l * r;
            case '/': return l / r;
            case '%': return l % r;
            default: exit(-2);
        }
    }
};

class AddExp: public BinaryExp{
public:
    explicit AddExp(string _sign) {
        sign = std::move(_sign);
        classType = AddExpType;
    }
    void check() override {
        assert(sign == "+" || sign == "-");
        cout << "AddExp check correct!" << endl;
        /*lch.check();
        rch.check();*/
    }
    int op(int l, int r) override {
        switch (sign[0]) {
            case '+': return l + r;
            case '-': return l - r;
            default: exit(-2);
        }
    }
};

class RelExp: public BinaryExp{
public:
    explicit RelExp(string _sign) {
        sign = std::move(_sign);
        classType = RelExpType;
    }
    void check() override {
        assert(sign == "<" || sign == ">" || sign == ">=" || sign == "<=");
        cout << "RelExp check correct!" << endl;
        /*lch.check();
        rch.check();*/
    }
    int op(int l, int r) override {
        if (sign == "<") return (l < r);
        else if (sign == ">") return (l > r);
        else if (sign == ">=") return (l >= r);
        else if (sign == "<=") return (l <= r);
        exit(-2);
    }
};

class EqExp: public BinaryExp{
public:
    explicit EqExp(string _sign) {
        sign = std::move(_sign);
        classType = EqExpType;
    }
    void check() override {
        /*assert(sign == "==" || sign == "!=");
        cout << "EqExp check correct!" << endl;
        lch.check();
        rch.check();*/
    }
    int op(int l, int r) override {
        switch(sign[0]) {
            case '=': return l == r;
            case '!': return l != r;
            default: exit(-2);
        }
    }
};

class LAndExp: public BinaryExp{
public:
    explicit LAndExp(string _sign) {
        sign = std::move(_sign);
        classType = LAndExpType;
    }
    void check() override {
        assert(sign == "&&");
        cout << "LAndExp check correct!" << endl;
        /*lch.check();
        rch.check();*/
    }
    int op(int l, int r) override {
        return (l && r);
    }
};

class LOrExp: public BinaryExp{
public:
    explicit LOrExp(string _sign) {
        sign = std::move(_sign);
        classType = LOrExpType;
    }
    void check() override  {
        assert(sign == "||");
        cout << "LOrExp check correct!" << endl;
        /*lch.check();
        rch.check();*/
    }
    int op(int l, int r) override {
        return (l || r);
    }
};

class AssignExp: public Node{
private:
    Node* ch[2] = {nullptr, nullptr};
    string sign;
public:
    AssignExp() {
        sign = ":=";
        classType = AssignExpType;
    }
    void check() override {
        assert(sign == ":=");
        cout << "AssignExp check correct!" << endl;
        /*lch.check();
        rch.check();*/
    }
    void traversal() override {
        string target, offset, left, right("%0");
        if (lch->getClassType() == VariableType && lch->getSize() == 1) {
            target = irTableList_1.getIrName(((Variable*)lch)->getName());
            if (lch->getType() != 0) {
                ((Variable*)lch)->getOffsetTree()->traversal();
                offset = irTableList_1.getTopTemIrName();
            }
        }
        else {
            cout << "assign error!" << endl;
            exit(10);
        }
        if (rch -> getClassType() == ReadValueType) {
            IR_1.add(new IrReadInteger(target));
        } else {
            if (rch->getClassType() == VariableType && rch->getSize() == 1 && rch->getType() == 0)
                left = irTableList_1.getIrName(((Variable*)rch)->getName());
            else {
                rch -> traversal();
                left = irTableList_1.getTopTemIrName();
            }
            if (offset.empty())
                IR_1.add(new IrBinaryOp(target, left, "+", right));
            else
                IR_1.add(new IrArrayAssign(target, offset, left));
        }
    }
    void setLch(Node* Lch) {
        if (lch)
            size -= lch->getSize();
        lch = Lch;
        if (lch)
            size += lch->getSize();
    }
    void setRch(Node* Rch) {
        if (rch)
            size -= rch->getSize();
        rch = Rch;
        if (rch)
            size += rch->getSize();
    }
    Node* optimize() override {
        return this;
    }
};
#endif //COMPILER_BINARY_EXP_H
