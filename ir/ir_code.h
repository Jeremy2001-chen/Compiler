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
        target = std::move(_tar);
    }
    void setSource(int index, string _so) {
        source[index] = std::move(_so);
    }
    string getSource(int index) {
        return source[index];
    }
};

class IrBinaryOp: public IrCode {
private:
    //string target, source[2];
    string sign;
public:
    IrBinaryOp(string _t, string _sc0, string _sign, string _sc1) {
        target = std::move(_t);
        source[0] = std::move(_sc0);
        source[1] = std::move(_sc1);
        sign = std::move(_sign);
        codeType = IrBinaryOpType;
    }

    string toString() override {
        return target + " = " + source[0] + " " + sign + " " + source[1];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
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
};

class IrUnaryOp: public IrCode {
private:
    //string target, source;
    string sign;
public:
    IrUnaryOp(string _te, string _sign, string _sc) {
        target = std::move(_te);
        sign = std::move(_sign);
        source[0] = std::move(_sc);
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
            mipsOutput -> push_back(new MipsAdd("sub", reg2, "$0", "$t0"));
            mipsTable -> setRegToMem(reg2, target);
        } else if (sign == "!") {
            mipsOutput -> push_back(new MipsAddI("seq", reg2, "$0", "$t0"));
            mipsTable -> setRegToMem(reg2, target);
        }
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        else
            return 0;
    }
};

class IrLabelLine: public IrCode {
private:
    string label;
public:
    explicit IrLabelLine(string _label) {
        label = std::move(_label);
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
        type = std::move(_type);
        name = std::move(_name);
        paraCount = std::move(_paraCount);
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
        type = std::move(_type);
        target = std::move(_name);
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
                mipsTable -> getRegFromAddress((*varToRegister)[target], target, 0, false);
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
        name = std::move(_name);
        source[0] = name;
        codeType = IrPushVariableType;
    }

    string toString() override {
        return "push var " + source[0];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg0 = mipsTable -> getRegFromMem("$t0", source[0]);
        int off = mipsTable -> getPushCnt();
        mipsOutput -> push_back(new MipsStore("sw", reg0, to_string(-(off << 2)) , "$sp"));
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
        source[0] = std::move(_name);
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
        string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
        string tar;
        if (source[1][0] >= '0' && source[1][0] <= '9')
            tar = source[0];
        else {
            reg1 = mipsTable -> getRegFromMem("$t0", source[1]);
            mipsOutput -> push_back(new MipsAddI("sll", "$t0", reg1, "2"));
            tar = "$t0";
        }
        reg0 = mipsTable -> getRegFromAddress("$t0", source[0], tar, true);
        int off = mipsTable -> getPushCnt();
        mipsOutput -> push_back(new MipsStore("sw", reg0, to_string(-(off << 2)), "$sp"));
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
        name = std::move(_name);
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
        source[0] = std::move(_sc);
        codeType = IrReturnStmtType;
    }

    string toString() override {
        if (source[0].empty())
            return "ret";
        return "ret " + source[0];
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (!source[0].empty())
            mipsTable -> getRegFromMemMust("$v0", source[0], true);
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
        target = std::move(_tar);
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
        name = std::move(_na);
        target = name;
        value = std::move(_va);
        codeType = IrVarDefineWithAssignType;
    }

    IrVarDefineWithAssign(bool _is, string _na, int _va) {
        isConst = _is;
        name = std::move(_na);
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
        name = std::move(_na);
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
        source[0] = std::move(sc0);
        source[1] = std::move(sc1);
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
        type = std::move(_type);
        source[0] = std::move(s0);
        source[1] = std::move(s1);
        label = std::move(_label);
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
        target = std::move(_na);
        offset = std::move(_off);
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
        if (varToRegister -> find(target) != varToRegister -> end())
            mipsTable -> getRegFromAddress((*varToRegister)[target], target, 0, false);
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
        target = std::move(_na);
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
            mipsTable -> getRegFromAddress((*varToRegister)[target], target, 0, false);
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
        source[0] = std::move(_ta);
        offset = std::move(_off);
        source[1] = offset;
        source[2] = std::move(_sc);
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
        string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
        string reg2 = mipsTable -> getRegFromMem("$t0", source[2]);
        if (source[1][0] >= '0' && source[1][0] <= '9')
            mipsTable -> setRegToMem(reg2, source[0], source[1]);
        else {
            mipsTable -> getRegFromMem(reg1, source[1]);
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
        target = std::move(_ta);
        source[0] = std::move(_sc);
        offset = std::move(_off);
        source[1] = offset;
        codeType = IrArrayGetType;
    }

    string toString() override {
        return target + " = " + source[0] + "[" + source[1] + "]";
    }

    //todo
    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg1 = mipsTable -> getRegFromMem("$t1", source[1]);
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        if (source[1][0] >= '0' && source[1][0] <= '9')
            reg2 = mipsTable -> getRegFromMem(reg2, source[0], source[1]);
        else {
            reg1 = mipsTable -> getRegFromMem(reg1, source[1]);
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
        target = std::move(_ta);
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
        str = std::move(_str);
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
        target = std::move(_ta);
        number = std::move(_num);
        codeType = IrNumberAssignType;
    }

    string toString() override {
        return target + " = " + number;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string reg2 = (varToRegister -> find(target) == varToRegister -> end()) ? "$t0" : (*varToRegister)[target];
        mipsOutput -> push_back(new MipsLi("li", reg2, number));
        mipsTable -> setRegToMem(reg2, target);
    }

    int defVar() override {
        if (varToRegister -> find(target) == varToRegister -> end())
            return mipsTable -> funInitStack(target, 1, false);
        return 0;
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
        target = std::move(_name);
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
        return "";
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
        target = std::move(_ta);
        source[0] = std::move(_so);
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
        target = std::move(_ta);
        source[0] = std::move(_so);
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
#endif //COMPILER_IR_CODE_H
