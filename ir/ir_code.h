//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_CODE_H
#define COMPILER_IR_CODE_H

#include <cstring>
#include <utility>
#include "../mips/mips_table.h"
#include "../mips/mips_output.h"
#include "../mips/mips_code.h"
#define IR_SOURCE (3)

using namespace std;

extern MipsOutput* mipsOutput;
extern MipsTable* mipsTable;

extern string IRNameTran(const string& str);
extern map<string, string>* varToRegister;

bool isNumber(string var) {
    return var[0] == '-' || var[0] >= '0' && var[0] <= '9';
}

class IrCode {
protected:
    IrType codeType;
    string target, source[IR_SOURCE];
public:
    virtual string toString() = 0;
    virtual void toMips() = 0;
    virtual int defVar() = 0;
    int getCodeType() const {
        return codeType;
    }
    string getTarget() const {
        return target;
    }
    void setTarget(string _tar) {
        target = _tar;
    }
    void setSource(int index, string _so) {
        source[index] = _so;
    }
    string getSource(int index) {
        return source[index];
    }
};

/* For MUL & DIV */
int log2(int x) {
    for (int i = 1; i < 32; ++ i)
        if (x <= (1ll << i)) {
            return i;
        }
    return 32;
}

int xSign(int x) {
    return x >= 0 ? 0 : -1;
}

class IrBinaryOp: public IrCode {
private:
    //string target, source[2];
    string sign;
public:
    IrBinaryOp(string _t, string _sc0, string _sign, string _sc1) {
        target = _t;
        source[0] = _sc0;
        source[1] = _sc1;
        sign = _sign;
        codeType = IrBinaryOpType;
    }

    string toString() override {
        return target + " = " + source[0] + " " + sign + " " + source[1];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
        if (isNumber(source[1])) {
            // addi $s0 $s0 100
            string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
            if (sign == "+")
                mipsOutput -> push_back(new MipsAddI("addi", reg2, reg0, source[1]));
            else if (sign == "-")
                mipsOutput -> push_back(new MipsAddI("subi", reg2, reg0, source[1]));
            else if (sign == "/") {
                reg2 = "$t9";
                int N = 32;
                int d = atoi(source[1].c_str());
                int d1 = d < 0 ? -d : d;
                int l = log2(d1);
                unsigned long long m = 1ull + ((1ull << (N + l - 1)) / d1);
                int m1 = m - (1ull << N);
                int d_sign = xSign(d);
                int sh_post = l - 1;
                mipsOutput -> push_back(new MipsLi("li", reg2, to_string(m1)));
                mipsOutput -> push_back(new MipsMul("mult", reg2, reg0));
                mipsOutput -> push_back(new MipsMF("mfhi", reg2));
                mipsOutput -> push_back(new MipsAdd("add", reg2, reg2, reg0));
                mipsOutput -> push_back(new MipsAddI("sra", reg2, reg2, to_string(sh_post)));
                mipsOutput -> push_back(new MipsAdd("slt", "$t1", reg0, "$0"));
                mipsOutput -> push_back(new MipsAdd("add", reg2, reg2, "$t1"));
                mipsOutput -> push_back(new MipsAddI("addi", "$t1", "$0", to_string(d_sign)));
                mipsOutput -> push_back(new MipsAdd("xor", reg2, reg2, "$t1"));
                mipsOutput -> push_back(new MipsAdd("sub", reg2, reg2, "$t1"));
            } else if (sign == "%") {
                reg2 = "$t9";
                int N = 32;
                int d = atoi(source[1].c_str());
                int d1 = d < 0 ? -d : d;
                int l = log2(d1);
                unsigned long long m = 1ull + ((1ull << (N + l - 1)) / d1);
                int m1 = m - (1ull << N);
                int d_sign = xSign(d);
                int sh_post = l - 1;
                mipsOutput -> push_back(new MipsLi("li", reg2, to_string(m1)));
                mipsOutput -> push_back(new MipsMul("mult", reg2, reg0));
                mipsOutput -> push_back(new MipsMF("mfhi", reg2));
                mipsOutput -> push_back(new MipsAdd("add", reg2, reg2, reg0));
                mipsOutput -> push_back(new MipsAddI("sra", reg2, reg2, to_string(sh_post)));
                mipsOutput -> push_back(new MipsAdd("slt", "$t1", reg0, "$0"));
                mipsOutput -> push_back(new MipsAdd("add", reg2, reg2, "$t1"));
                mipsOutput -> push_back(new MipsAddI("addi", "$t1", "$0", to_string(d_sign)));
                mipsOutput -> push_back(new MipsAdd("xor", reg2, reg2, "$t1"));
                mipsOutput -> push_back(new MipsAdd("sub", reg2, reg2, "$t1"));
                mipsOutput -> push_back(new MipsLi("li", "$t1", source[1]));
                mipsOutput -> push_back(new MipsMul("mult", reg2, "$t1"));
                mipsOutput -> push_back(new MipsMF("mflo", reg2));
                mipsOutput -> push_back(new MipsAdd("sub", reg2, reg0, reg2));
            }
            mipsTable -> setRegToMem(reg2, target);
            return ;
        }
        string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        if (sign == "+")
            mipsOutput -> push_back(new MipsAdd("add", reg2, reg0, reg1));
        else if (sign == "-")
            mipsOutput -> push_back(new MipsAdd("sub", reg2, reg0, reg1));
        else if (sign == "*") {
            mipsOutput -> push_back(new MipsMul("mult", reg0, reg1));
            mipsOutput -> push_back(new MipsMF("mflo", reg2));
        } else if (sign == "/") {
            mipsOutput -> push_back(new MipsMul("div", reg0, reg1));
            mipsOutput -> push_back(new MipsMF("mflo", reg2));
        } else if (sign == "%") {
            mipsOutput -> push_back(new MipsMul("div", reg0, reg1));
            mipsOutput -> push_back(new MipsMF("mfhi", reg2));
        } else if (sign == "<") {
            mipsOutput -> push_back(new MipsAdd("slt", reg2, reg0, reg1));
        } else if (sign == ">") {
            mipsOutput -> push_back(new MipsAdd("sgt", reg2, reg0, reg1));
        } else if (sign == ">=") {
            mipsOutput -> push_back(new MipsAdd("sge", reg2, reg0, reg1));
        } else if (sign == "<=") {
            mipsOutput -> push_back(new MipsAdd("sle", reg2, reg0, reg1));
        } else if (sign == "==") {
            mipsOutput -> push_back(new MipsAdd("seq", reg2, reg1, reg0));
        } else if (sign == "!=") {
            mipsOutput -> push_back(new MipsAdd("sne", reg2, reg1, reg0));
        } else if (sign == "&&") {
            mipsOutput -> push_back(new MipsAdd("and", reg2, reg0, reg1));
        } else if (sign == "||") {
            mipsOutput -> push_back(new MipsAdd("or", reg2, reg0, reg1));
        }
        mipsTable -> setRegToMem(reg2, target);
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        else
            return 0;
    }

    string getSign() {
        return sign;
    }
};

class IrUnaryOp: public IrCode {
private:
    //string target, source;
    string sign;
public:
    IrUnaryOp(string _te, string _sign, string _sc) {
        target = _te;
        sign = _sign;
        source[0] = _sc;
        codeType = IrUnaryOpType;
    }

    string toString() override {
        return target + " = " + sign + " " + source[0];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        if (sign == "+") {
            mipsTable -> setRegToMem(reg0, target);
        } else if (sign == "-") {
            mipsOutput -> push_back(new MipsAdd("sub", reg2, "$0", reg0));
            mipsTable -> setRegToMem(reg2, target);
        } else if (sign == "!") {
            mipsOutput -> push_back(new MipsAddI("seq", reg2, "$0", reg0));
            mipsTable -> setRegToMem(reg2, target);
        }
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        else
            return 0;
    }

    string getSign() const {
        return sign;
    }
};

class IrLabelLine: public IrCode {
private:
    string label;
public:
    explicit IrLabelLine(string _label) {
        label = _label;
        codeType = IrLabelLineType;
    }

    string toString() override {
        return label + ":";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsOutput -> push_back(new MipsLabel(label));
    }

    int defVar() override {
        return 0;
    }

    string getLabel() const {
        return label;
    }
};

class IrFunDefine: public IrCode {
private:
    string type, paraCount, name;
public:
    IrFunDefine(string _type, string _paraCount, string _name) {
        type = _type;
        name = _name;
        paraCount = _paraCount;
        codeType = IrFunDefineType;
    }

    string toString() override {
        return type + " " + name + "(" + paraCount + ")";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> setLayer(1);
        /*
        int cnt = atoi(paraCount.c_str()) + 1;
        cnt *= 4;
        mipsOutput -> push_back(new MipsAddI("subi", "$sp", "$sp", to_string(cnt)));
         */
    }

    int defVar() override {
        return 0;
    }

    string getName() const {
        return name;
    }
};

class IrParaDefine: public IrCode {
private:
    string type;
public:
    IrParaDefine(string _type, string _name) {
        type = _type;
        target = _name;
        codeType = IrParaDefineType;
    }

    string toString() override {
        return "para " + type + " " + target;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        //mipsTable -> setTopName(name);
        bool isAdd = (type == "arr");
        if (varToRegister -> find(target) != varToRegister -> end()) {
            if (isAdd)
                mipsTable -> getRegFromAddress((*varToRegister)[target], target, to_string(0), false);
            else
                mipsTable -> getRegFromMemMust((*varToRegister)[target], target, false);
        }
    }
    int defVar() override {
        bool isAdd = (type == "arr");
        cout << "check : " << target << endl;
        //if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, isAdd);
        //return 0;
    }
};

class IrPushVariable: public IrCode {
private:
    string name;
public:
    explicit IrPushVariable(string _name) {
        name = _name;
        source[0] = name;
        codeType = IrPushVariableType;
    }

    string toString() override {
        return "push var " + source[0];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0;
        if (isNumber(source[0])) {
            int number = atoi(source[0].c_str());
            if (number == 0)
                reg0 = "$0";
            else {
                reg0 = "$t0";
                mipsOutput -> push_back(new MipsLi("li", reg0, source[0]));
            }
        } else
            reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
        int off = mipsTable -> getPushCnt();
//        if (off > 4)
            mipsOutput -> push_back(new MipsStore("sw", reg0, to_string(-(off << 2)) , "$sp"));
//        else {
//            string reg = "$a" + to_string(off - 1);
//            mipsOutput -> push_back(new MipsAdd("add", reg, reg0, "$0"));
//        }
    }

    int defVar() override {
        return 0;
    }
};

class IrPushArray: public IrCode {
private:
    //string name;
    string offset;
public:
    IrPushArray(string _name, string _offset) {
        source[0] = _name;
        offset = _offset;
        source[1] = offset;
        codeType = IrPushArrayType;
    }

    string toString() override {
        return "push array " + source[0] + "[" + source[1] + "]";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
        string reg1;
        string tar;
        if (isNumber(source[1]))
            tar = source[1];
        else {
            reg1 = mipsTable -> getRegFromMem("$t0", source[1]);
            mipsOutput -> push_back(new MipsAddI("sll", "$t0", reg1, "2"));
            tar = "$t0";
        }
        reg0 = mipsTable -> getRegFromAddress("$t0", source[0], tar, true);
        int off = mipsTable -> getPushCnt();
//        if (off > 4)
            mipsOutput -> push_back(new MipsStore("sw", reg0, to_string(-(off << 2)), "$sp"));
//        else {
//            string reg = "$a" + to_string(off - 1);
//            mipsOutput -> push_back(new MipsAdd("add", reg, reg0, "$0"));
//        }
    }

    int defVar() override {
        return 0;
    }
};

class IrCallFunction: public IrCode {
private:
    string name;
    int varCnt;
public:
    explicit IrCallFunction(string _name, int _cnt) {
        name = _name;
        varCnt = _cnt;
        codeType = IrCallFunctionType;
    }

    string toString() override {
        return "call " + name;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        int off = mipsTable -> getPushCnt();
        mipsOutput -> push_back(new MipsStore("sw", "$ra", to_string(-(off << 2)), "$sp"));
        mipsOutput -> push_back(new MipsJLabel("jal", name));
        mipsOutput -> push_back(new MipsStore("lw", "$ra", to_string(-(off << 2)), "$sp"));
        mipsTable -> setPushCnt(-(varCnt + 1));
    }

    int defVar() override {
        return 0;
    }
};

class IrReturnStmt: public IrCode {
private:
    //string source;
public:
    IrReturnStmt() {
        source[0] = "";
        codeType = IrReturnStmtType;
    }

    IrReturnStmt(string _sc) {
        source[0] = _sc;
        codeType = IrReturnStmtType;
    }

    string toString() override {
        if (source[0].empty())
            return "ret";
        return "ret " + source[0];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (!source[0].empty()) {
            if (isNumber(source[0])) {
                int number = atoi(source[0].c_str());
                if (number == 0)
                    mipsOutput -> push_back(new MipsAdd("add", "$v0", "$0", "$0"));
                else
                    mipsOutput -> push_back(new MipsLi("li", "$v0", source[0]));
            } else
                mipsTable -> getRegFromMemMust("$v0", source[0], true);
        }
//        int cnt = mipsTable -> getTopParaCnt();
//        mipsOutput -> push_back(new MipsAddI("addi", "$sp", "$sp", to_string(cnt)));
        //mipsOutput -> push_back(new MipsJRegister("jr", "$ra"));
    }

    int defVar() override {
        return 0;
    }
};

class IrReturnValStmt: public IrCode {
private:
    //string target;
public:
    explicit IrReturnValStmt(string _tar) {
        target = _tar;
        codeType = IrReturnValStmtType;
    }

    string toString() override {
        return target + " = RET";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> setRegToMem("$v0", target);
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }
};

class IrVarDefineWithAssign: public IrCode {
private:
    bool isConst;
    string name, value;
public:
    IrVarDefineWithAssign(bool _is, string _na, string _va) {
        isConst = _is;
        name = _na;
        target = name;
        value = _va;
        codeType = IrVarDefineWithAssignType;
    }

    IrVarDefineWithAssign(bool _is, string _na, int _va) {
        isConst = _is;
        name = _na;
        target = name;
        value = to_string(_va);
        codeType = IrVarDefineWithAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "var int " + target + " = " + value;
    }

    string getName() const {
        return target;
    }

    int getValue() const {
        return atoi(value.c_str());
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        mipsOutput -> push_back(new MipsLi("li", reg2, value));
        mipsTable -> setRegToMem(reg2, target);
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }
};

class IrVarDefineWithOutAssign: public IrCode {
private:
    bool isConst;
    string name;
public:
    IrVarDefineWithOutAssign(bool _is, string _na) {
        isConst = _is;
        name = _na;
        target = name;
        codeType = IrVarDefineWithOutAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "var int " + target;
    }

    string getName() const {
        return target;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }
};

class IrCmpStmt: public IrCode {
public:
    IrCmpStmt(string sc0, string sc1) {
        source[0] = sc0;
        source[1] = sc1;
        codeType = IrCmpStmtType;
    }

    string toString() override {
        return "cmp " + source[0] + ", " + source[1];
    }

    string* getSources() {
        return source;
    };

    void toMips() override {

    }

    int defVar() override {
        return 0;
    }
};

class IrBranchStmt: public IrCode {
private:
    string type, label;
public:
    IrBranchStmt(string _type, string s0, string s1, string _label) {
        type = _type;
        source[0] = s0;
        source[1] = s1;
        label = _label;
        codeType = IrBranchStmtType;
    }

    string toString() override {
        return type + " " + source[0] + " " + source[1] + " " + label;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
        string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
        mipsOutput -> push_back(new MipsBranch(type, reg0, reg1, label));
    }

    int defVar() override {
        return 0;
    }

    string getLabel() {
        return label;
    }

    void setLabel(string name) {
        label = name;
    }

    string getType() {
        return type;
    }
};

class IrGotoStmt: public IrCode {
private:
    string label;
public:
    explicit IrGotoStmt(string _label) {
        label = _label;
        codeType = IrGotoStmtType;
    }

    string toString() override {
        return "goto " + label;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsOutput -> push_back(new MipsJLabel("j", label));
    }

    int defVar() override {
        return 0;
    }

    string getLabel() const {
        return label;
    }
};

class IrArrayDefineWithOutAssign: public IrCode {
private:
    bool isConst;
    string offset;
public:
    IrArrayDefineWithOutAssign(bool _is, string _na, string _off) {
        isConst = _is;
        target = _na;
        offset = _off;
        codeType = IrArrayDefineWithOutAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "arr int " + target + "[" + offset + "]";
    }

    string getName() {
        return target;
    }

    int getSize() const {
        return atoi(offset.c_str());
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (varToRegister -> find(target) != varToRegister -> end()) {
            mipsTable -> getRegFromAddress((*varToRegister)[target], target, to_string(0), false);
        }
    }

    int defVar() override {
        int tot = atoi(offset.c_str());
        return mipsTable -> funInitStack(target, tot, false);
    }
};

class IrArrayDefineWithAssign: public IrCode {
private:
    bool isConst;
    //string name;
    int size;
    vector<int>* values;
public:
    IrArrayDefineWithAssign(bool _is, string _na, int _size, vector<int>* var) {
        isConst = _is;
        target = _na;
        values = var;
        size = _size;
        codeType = IrArrayDefineWithAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        tmp = tmp + "arr int " + target + "[" + to_string((*values).size()) + "] =";
        for (int i : *values)
            tmp += " " + to_string(i);
        return tmp;
    }

    string getName() {
        return target;
    }

    int getSize() const {
        return size;
    }

    vector<int>* getValues() {
        return values;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (varToRegister -> find(target) != varToRegister -> end())
            mipsTable -> getRegFromAddress((*varToRegister)[target], target, to_string(0), false);
        //for temporary array with assign, should not call toMips!!!
        exit(23456);
    }

    int defVar() override {
        return mipsTable -> funInitStack(target, size, false);
    }
};

class IrArrayAssign: public IrCode {
private:
    //string target, offset, source;
    //string name;
    string offset;
public:
    IrArrayAssign(string _ta, string _off, string _sc) {
        source[0] = _ta;
        offset = _off;
        source[1] = offset;
        source[2] = _sc;
        codeType = IrArrayAssignType;
    }

    string toString() override {
        return source[0] + "[" + source[1] + "] = " + source[2];
    }

    string getOffset() {
        return source[1];
    }

    //todo
    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg2;
        if (isNumber(source[2])) {
            int number = atoi(source[2].c_str());
            if (number == 0)
                reg2 = "$0";
            else {
                reg2 = "$t0";
                mipsOutput -> push_back(new MipsLi("li", "$t0", source[2]));
            }
        } else
            reg2 = mipsTable -> getRegFromMem("$t0", source[2]);
        if (isNumber(source[1]))
            mipsTable -> setRegToMem(reg2, source[0], source[1]);
        else {
            string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
            mipsOutput -> push_back(new MipsAddI("sll", "$t1", reg1, "2"));
            mipsTable -> setRegToMem(reg2, source[0], "$t1");
        }
    }

    int defVar() override {
        return 0;
    }
};

class IrArrayGet: public IrCode {
private:
    //string target, source, offset;
    //string name;
    string offset;
public:
    IrArrayGet(string _ta, string _sc, string _off) {
        target = _ta;
        source[0] = _sc;
        offset = _off;
        source[1] = offset;
        codeType = IrArrayGetType;
    }

    string toString() override {
        return target + " = " + source[0] + "[" + source[1] + "]";
    }

    //todo
    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        if (isNumber(source[1]))
            reg2 = mipsTable -> getRegFromMem(reg2, source[0], source[1]);
        else {
            string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
            mipsOutput -> push_back(new MipsAddI("sll", "$t1", reg1, "2"));
            reg2 = mipsTable -> getRegFromMem(reg2, source[0], "$t1");
        }
        mipsTable -> setRegToMem(reg2, target);
        //cout << toString() << endl;
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }

    string getTarget() {
        return target;
    }
};

class IrReadInteger: public IrCode {
private:
    //string target;
public:
    explicit IrReadInteger(string _ta) {
        target = _ta;
        codeType = IrReadIntegerType;
    }

    string toString() override {
        return target + " = getint()";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsOutput -> push_back(new MipsLi("li", "$v0", "5"));
        mipsOutput -> push_back(new MipsSyscall());
        mipsTable -> setRegToMem("$v0", target);
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }
};

class IrPrintInteger: public IrCode {
private:
    //string source;
public:
    explicit IrPrintInteger(string _sc) {
        source[0] = _sc;
        codeType = IrPrintIntegerType;
    }

    string toString() override {
        return "printInt " + source[0];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (isNumber(source[0])) {
            int number = atoi(source[0].c_str());
            if (number == 0)
                mipsOutput -> push_back(new MipsAdd("add", "$a0", "$0", "0"));
            else
                mipsOutput -> push_back(new MipsLi("li", "$a0", source[0]));
        } else
            mipsTable -> getRegFromMemMust("$a0", source[0], true);
        mipsOutput -> push_back(new MipsLi("li", "$v0", "1"));
        mipsOutput -> push_back(new MipsSyscall());
    }

    int defVar() override {
        return 0;
    }
};

class IrPrintString: public IrCode {
private:
    string str;
public:
    explicit IrPrintString(string _str) {
        str = _str;
        codeType = IrPrintStringType;
    }

    string toString() override {
        return "printStr \"" + str + "\"";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string label = mipsTable -> getLabel(str);
        mipsOutput -> push_back(new MipsLa("la", "$a0", label));
        mipsOutput -> push_back(new MipsAddI("addi", "$v0", "$0", "4"));
        mipsOutput -> push_back(new MipsSyscall());
    }

    string getStr() {
        return str;
    }

    int defVar() override {
        return 0;
    }
};

class IrNumberAssign: public IrCode {
private:
    //string target, number;
    string number;
public:
    IrNumberAssign(string _ta, string _num) {
        target = _ta;
        number = _num;
        codeType = IrNumberAssignType;
    }

    string toString() override {
        return target + " = " + number;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        if (number == "0")
            mipsTable -> setRegToMem("$0", target);
        else {
            mipsOutput -> push_back(new MipsLi("li", reg2, number));
            mipsTable -> setRegToMem(reg2, target);
        }
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }

    int getNumber() const {
        return atoi(number.c_str());
    }
};

class IrFunEnd: public IrCode {
private:
    int cnt;
public:
    IrFunEnd(int _cnt) {
        cnt = _cnt;
        codeType = IrFunEndType;
    }

    string toString() override {
        return "^^EndFunction^^";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> setLayer(-1);
    }

    int defVar() override {
        return 0;
    }
};

class IrExit: public IrCode {
public:
    IrExit() {
        codeType = IrExitType;
    }

    string toString() override {
        return "exit";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsOutput -> push_back(new MipsLi("li", "$v0", to_string(10)));
        mipsOutput -> push_back(new MipsSyscall());
    }

    int defVar() override {
        return 0;
    }
};

class IrPhi: public IrCode {
private:
    vector<string>* from;
    vector<int>* blockNum;
public:
    IrPhi(string _name) {
        codeType = IrPhiType;
        target = _name;
        from = new vector<string>();
        blockNum = new vector<int>();
    }

    void putVar(string var, int num) {
        from -> push_back(var);
        blockNum -> push_back(num);
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
    }

    string toString() override {
//        return "";
        string ret = target + " = Phi(";
        if (from -> size() > 0) {
            ret = ret + (*from)[0];
            for (int i = 1; i < from -> size(); ++ i)
                ret = ret + ", " + (*from)[i];
        }
        ret += ")";
        return ret;
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }

    vector<string>* getFrom() {
        return from;
    }

    vector<int>* getBlockNum() {
        return blockNum;
    }
};

class IrPhiAssign: public IrCode {
public:
    IrPhiAssign(string _ta, string _so) {
        codeType = IrPhiAssignType;
        target = _ta;
        source[0] = _so;
    }

    string toString() override {
        string ret = target + " <- " + source[0];
        return ret;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        //string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        //reg2 = mipsTable -> getRegFromMem(reg2, source[0]);
        //mipsTable -> setRegToMem(reg2, target);
    }

    int defVar() override {
        return 0;
    }
};

class IrMove: public IrCode {
private:
    bool type;
public:
    IrMove(string _ta, string _so, bool _def) {
        codeType = IrMoveType;
        target = _ta;
        source[0] = _so;
        type = _def;
    }

    string toString() override {
        string ret = target + " = + " + source[0];
        return ret;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        reg2 = mipsTable -> getRegFromMem(reg2, source[0]);
        mipsTable -> setRegToMem(reg2, target);
    }

    int defVar() override {
        //if (type)
        //    return mipsTable -> funInitStack(target, 1, false);
        return 0;
    }
};

class IrNop: public IrCode {
public:
    IrNop() {
        source[0] = source[1] = "";
        codeType = IrNopType;
    }

    string toString() override {
        return "nop";
    }

    void toMips() override {

    }

    int defVar() override {
        return 0;
    }
};
#endif //COMPILER_IR_CODE_H
