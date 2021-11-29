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
        if (!globalVarDecl && (!lch -> getConstType() || !rch -> getConstType()))
            return this;
        ConstValue* lchConst = (ConstValue*)lch, *rchConst = (ConstValue*)rch;
        Number* number = new Number(op(lchConst->getValue(), rchConst->getValue()));
        return number;
    }
    void traversal() override {
        if (classType == AddExpType || classType == MulType || classType == RelExpType || classType == EqExpType) {
            string target, left, right;
            if (lch->getClassType() == VariableType && lch->getSize() == 1 && ((Variable*)lch) -> getIsArray() == 0)
                left = irTableList_1.getIrName(((Variable*)lch)->getName());
            else {
                lch -> traversal();
                left = irTableList_1.getTopTemIrName();
            }
            if (rch->getClassType() == VariableType && rch->getSize() == 1 && ((Variable*)rch) -> getIsArray() == 0)
                right =irTableList_1.getIrName(((Variable*)rch)->getName());
            else {
                rch -> traversal();
                right = irTableList_1.getTopTemIrName();
            }
            target = irTableList_1.allocTem();
            IR_1.add(new IrBinaryOp(target, left, sign, right));
        } else {
            string target, left, right;
            if (lch->getClassType() == VariableType && lch->getSize() == 1 && ((Variable*)lch) -> getIsArray() == 0)
                left = irTableList_1.getIrName(((Variable*)lch)->getName());
            else {
                lch -> traversal();
                left = irTableList_1.getTopTemIrName();
            }
            string br1 = irTableList_1.allocBranch(), br2 = irTableList_1.allocBranch();
            switch(sign[0]) {
                case '&':
                    IR_1.add(new IrBranchStmt("beq", left, "%0", br1));
                    break;
                case '|':
                    IR_1.add(new IrBranchStmt("bgt", left, "%0", br1));
                    break;
                default: exit(-2);
            }
            if (rch->getClassType() == VariableType && rch->getSize() == 1 && ((Variable*)rch) -> getIsArray() == 0)
                right = irTableList_1.getIrName(((Variable*)rch)->getName());
            else {
                rch -> traversal();
                right = irTableList_1.getTopTemIrName();
            }
            switch(sign[0]) {
                case '&':
                    IR_1.add(new IrBranchStmt("beq", right, "%0", br1));
                    break;
                case '|':
                    IR_1.add(new IrBranchStmt("bgt", right, "%0", br1));
                    break;
                default: exit(-2);
            }
            string num[2] = {sign[0] == '&' ? "1" : "0", sign[0] == '&' ? "0" : "1"};
            target = irTableList_1.allocTem();
            IR_1.add(new IrNumberAssign(target, num[0]));
            IR_1.add(new IrGotoStmt(br2));
            IR_1.add(new IrLabelLine(br1));
            IR_1.add(new IrNumberAssign(target, num[1]));
            IR_1.add(new IrLabelLine(br2));
        }
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
            if (((Variable*)lch)->getIsArray() != 0) {
                ((Variable*)lch)->getOffsetTree()->traversal();
                offset = irTableList_1.getTopTemIrName();
            }
        }
        else {
            cout << "assign error!" << endl;
            exit(10);
        }
        string tem = (offset.empty() ? target : irTableList_1.allocTem());
        if (rch -> getClassType() == ReadValueType) {
            IR_1.add(new IrReadInteger(tem));
        } else if (rch -> getClassType() == NumberType) {
            IR_1.add(new IrNumberAssign(tem, to_string(((ConstValue*)rch) -> getValue())));
        } else if (rch -> getClassType() == VariableType) {
            Variable* rt = (Variable*) rch;
            string rtName = irTableList_1.getIrName(rt -> getName());
            if (rt -> getIsArray()) {
                Node* offset = rt -> getOffsetTree();
                offset -> traversal();
                string off = irTableList_1.getTopTemIrName();
                IR_1.add(new IrArrayGet(tem, rtName, off));
            } else {
                IR_1.add(new IrBinaryOp(tem, rtName, "+", "%0"));
            }
        } else {
            rch -> traversal();
            IR_1.add(new IrBinaryOp(tem, irTableList_1.getTopTemIrName(), "+", "%0"));
        }


        if (!offset.empty()) {
            IR_1.add(new IrArrayAssign(target, offset, tem));
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
