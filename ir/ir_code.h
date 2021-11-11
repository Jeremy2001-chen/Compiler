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

using namespace std;

extern MipsOutput* mipsOutput;
extern MipsTable* mipsTable;

extern string IRNameTran(const string& str);

class IrCode {
protected:
    IrType codeType;
public:
    virtual string toString() = 0;
    virtual void toMips() = 0;
    virtual int defVar() = 0;
    int getCodeType() const {
        return codeType;
    }
};

class IrBinaryOp: public IrCode {
private:
    string target, source[2];
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
        mipsTable -> getRegFromMem("$t0", source[0]);
        mipsTable -> getRegFromMem("$t1", source[1]);
        if (sign == "+")
            mipsOutput -> push_back(new MipsAdd("add", "$t2", "$t0", "$t1"));
        else if (sign == "-")
            mipsOutput -> push_back(new MipsAdd("sub", "$t2", "$t0", "$t1"));
        else if (sign == "*") {
            mipsOutput -> push_back(new MipsMul("mult", "$t0", "$t1"));
            mipsOutput -> push_back(new MipsMF("mflo", "$t2"));
        } else if (sign == "/") {
            mipsOutput -> push_back(new MipsMul("div", "$t0", "$t1"));
            mipsOutput -> push_back(new MipsMF("mflo", "$t2"));
        } else if (sign == "%") {
            mipsOutput -> push_back(new MipsMul("div", "$t0", "$t1"));
            mipsOutput -> push_back(new MipsMF("mfhi", "$t2"));
        } else if (sign == "<") {
            mipsOutput -> push_back(new MipsAdd("slt", "$t2", "$t0", "$t1"));
        } else if (sign == ">") {
            mipsOutput -> push_back(new MipsAdd("sgt", "$t2", "$t0", "$t1"));
        } else if (sign == ">=") {
            mipsOutput -> push_back(new MipsAdd("sge", "$t2", "$t0", "$t1"));
        } else if (sign == "<=") {
            mipsOutput -> push_back(new MipsAdd("sle", "$t2", "$t0", "$t1"));
        } else if (sign == "==") {
            mipsOutput -> push_back(new MipsAdd("seq", "$t2", "$t1", "$t0"));
        } else if (sign == "!=") {
            mipsOutput -> push_back(new MipsAdd("sne", "$t2", "$t1", "$t0"));
        } else if (sign == "&&") {
            mipsOutput -> push_back(new MipsAdd("and", "$t2", "$t0", "$t1"));
        } else if (sign == "||") {
            mipsOutput -> push_back(new MipsAdd("or", "$t2", "$t0", "$t1"));
        }
        mipsTable -> setRegToMem("$t2", target);
    }

    int defVar() override {
        return mipsTable -> funInitStack(target, 1, false);
    }

    string getTarget() const {
        return target;
    }
};

class IrUnaryOp: public IrCode {
private:
    string target, source;
    string sign;
public:
    IrUnaryOp(string _te, string _sign, string _sc) {
        target = std::move(_te);
        sign = std::move(_sign);
        source = std::move(_sc);
        codeType = IrUnaryOpType;
    }

    string toString() override {
        return target + " = " + sign + " " + source;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> getRegFromMem("$t0", source);
        if (sign == "+") {
            mipsTable -> setRegToMem("$t0", target);
        } else if (sign == "-") {
            mipsOutput -> push_back(new MipsAdd("sub", "$t1", "$0", "$t0"));
            mipsTable -> setRegToMem("$t1", target);
        } else if (sign == "!") {
            mipsOutput -> push_back(new MipsAddI("seq", "$t1", "$0", "$t0"));
            mipsTable -> setRegToMem("$t1", target);
        }
    }

    int defVar() override {
        return mipsTable -> funInitStack(target, 1, false);
    }

    string getTarget() const {
        return target;
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
};

class IrParaDefine: public IrCode {
private:
    string type, name;
public:
    IrParaDefine(string _type, string _name) {
        type = std::move(_type);
        name = std::move(_name);
        codeType = IrParaDefineType;
    }

    string toString() override {
        return "para " + type + " " + name;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        //mipsTable -> setTopName(name);
    }

    int defVar() override {
        if (type == "var" || type == "reg")
            return mipsTable -> funInitStack(name, 1, false);
        else
            return mipsTable -> funInitStack(name, 1, true);
    }
};

class IrPushVariable: public IrCode {
private:
    string name;
public:
    explicit IrPushVariable(string _name) {
        name = std::move(_name);
        codeType = IrPushVariableType;
    }

    string toString() override {
        return "push var " + name;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> getRegFromMem("$t0", name);
        int off = mipsTable -> getPushCnt();
        cout << "push error: " << off << endl;
        mipsOutput -> push_back(new MipsStore("sw", "$t0", to_string(-(off << 2)) , "$sp"));
    }

    int defVar() override {
        return 0;
    }
};

class IrPushArray: public IrCode {
private:
    string name;
    string offset;
public:
    IrPushArray(string _name, string _offset) {
        name = std::move(_name);
        offset = _offset;
        codeType = IrPushArrayType;
    }

    string toString() override {
        return "push array " + name + "[" + offset + "]";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        string tar;
        if (offset[0] >= '0' && offset[0] <= '9')
            tar = offset;
        else {
            mipsTable -> getRegFromMem("$t0", offset);
            mipsOutput -> push_back(new MipsAdd("sll", "$t0", "$t0", "2"));
            tar = "$t0";
        }
        mipsTable -> getRegFromAddress("$t1", name, tar);
        int off = mipsTable -> getPushCnt();
        mipsOutput -> push_back(new MipsStore("sw", "$t1", to_string(-(off << 2)), "$sp"));
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
    string source;
public:
    IrReturnStmt() {
        source = "";
        codeType = IrReturnStmtType;
    }

    explicit IrReturnStmt(string _sc) {
        source = std::move(_sc);
    }

    string toString() override {
        if (source.empty())
            return "ret";
        return "ret " + source;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (!source.empty())
            mipsTable -> getRegFromMem("$v0", source);
        int cnt = mipsTable -> getTopParaCnt();
        mipsOutput -> push_back(new MipsAddI("addi", "$sp", "$sp", to_string(cnt)));
        mipsOutput -> push_back(new MipsJRegister("jr", "$ra"));
    }

    int defVar() override {
        return 0;
    }
};

class IrReturnValStmt: public IrCode {
private:
    string target;
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
        return mipsTable -> funInitStack(target, 1, false);
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
        value = std::move(_va);
        codeType = IrVarDefineWithAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "var int " + name + " = " + value;
    }

    string getName() const {
        return name;
    }

    int getValue() const {
        return atoi(value.c_str());
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsOutput -> push_back(new MipsLi("li", "$t0", value));
        mipsTable -> setRegToMem("$t0", name);
    }

    int defVar() override {
        return mipsTable -> funInitStack(name, 1, false);
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
        codeType = IrVarDefineWithOutAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "var int " + name;
    }

    string getName() const {
        return name;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
    }

    int defVar() override {
        return mipsTable -> funInitStack(name, 1, false);
    }
};

class IrCmpStmt: public IrCode {
private:
    string source[2];
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
    string type, source[2], label;
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
        mipsTable -> getRegFromMem("$t0", source[0]);
        mipsTable -> getRegFromMem("$t1", source[1]);
        mipsOutput -> push_back(new MipsBranch(type, "$t0", "$t1", label));
    }

    int defVar() override {
        return 0;
    }
};

class IrGotoStmt: public IrCode {
private:
    string label;
public:
    explicit IrGotoStmt(string _label) {
        label = std::move(_label);
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
};

class IrArrayDefineWithOutAssign: public IrCode {
private:
    bool isConst;
    string name, offset;
public:
    IrArrayDefineWithOutAssign(bool _is, string _na, string _off) {
        isConst = _is;
        name = std::move(_na);
        offset = std::move(_off);
        codeType = IrArrayDefineWithOutAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "arr int " + name + "[" + offset + "]";
    }

    string getName() {
        return name;
    }

    int getSize() const {
        return atoi(offset.c_str());
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
    }

    int defVar() override {
        int tot = atoi(offset.c_str());
        return mipsTable -> funInitStack(name, tot, false);
    }
};

class IrArrayDefineWithAssign: public IrCode {
private:
    bool isConst;
    string name;
    int size;
    vector<int>* values;
public:
    IrArrayDefineWithAssign(bool _is, string _na, int _size, vector<int>* var) {
        isConst = _is;
        name = std::move(_na);
        values = var;
        size = _size;
        codeType = IrArrayDefineWithAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        tmp = tmp + "arr int " + name + "[" + to_string((*values).size()) + "] =";
        for (int i : *values)
            tmp += " " + to_string(i);
        return tmp;
    }

    string getName() {
        return name;
    }

    int getSize() const {
        return size;
    }

    vector<int>* getValues() {
        return values;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        //for temporary array with assign, should not call toMips!!!
        exit(23456);
    }

    int defVar() override {
        return mipsTable -> funInitStack(name, size, false);
    }
};

class IrArrayAssign: public IrCode {
private:
    string target, offset, source;
public:
    IrArrayAssign(string _ta, string _off, string _sc) {
        target = std::move(_ta);
        offset = std::move(_off);
        source = std::move(_sc);
        codeType = IrArrayAssignType;
    }

    string toString() override {
        return target + "[" + offset + "] = " + source;
    }

    string getTarget() {
        return target;
    }

    string getOffset() {
        return offset;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> getRegFromMem("$t0", source);
        if (offset[0] >= '0' && offset[0] <= '9')
            mipsTable -> setRegToMem("$t0", target, offset);
        else {
            mipsTable -> getRegFromMem("$t1", offset);
            mipsOutput -> push_back(new MipsAddI("sll", "$t1", "$t1", "2"));
            mipsTable -> setRegToMem("$t0", target, "$t1");
        }
    }

    int defVar() override {
        return 0;
    }
};

class IrArrayGet: public IrCode {
private:
    string target, source, offset;
public:
    IrArrayGet(string _ta, string _sc, string _off) {
        target = std::move(_ta);
        source = std::move(_sc);
        offset = std::move(_off);
        codeType = IrArrayGetType;
    }

    string toString() override {
        return target + " = " + source + "[" + offset + "]";
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        if (offset[0] >= '0' && offset[0] <= '9')
            mipsTable -> setRegToMem("$t0", target, to_string(atoi(offset.c_str()) << 2));
        else {
            mipsTable -> getRegFromMem("$t1", offset);
            mipsOutput -> push_back(new MipsAddI("sll", "$t1", "$t1", "2"));
            mipsTable -> getRegFromMem("$t0", source, "$t1");
        }
        mipsTable -> setRegToMem("$t0", target);
    }

    int defVar() override {
        return mipsTable -> funInitStack(target, 1, false);
    }
};

class IrReadInteger: public IrCode {
private:
    string target;
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
        return mipsTable -> funInitStack(target, 1, false);
    }
};

class IrPrintInteger: public IrCode {
private:
    string source;
public:
    explicit IrPrintInteger(string _sc) {
        source = std::move(_sc);
        codeType = IrPrintIntegerType;
    }

    string toString() override {
        return "printInt " + source;
    }

    void toMips() override {
        mipsOutput -> push_back(new MipsNote(toString()));
        mipsTable -> getRegFromMem("$a0", source);
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
    string target, number;
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
        mipsOutput -> push_back(new MipsLi("li", "$t0", number));
        mipsTable -> setRegToMem("$t0", target);
    }

    int defVar() override {
        return mipsTable -> funInitStack(target, 1, false);
    }

    string getTarget() const {
        return target;
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
#endif //COMPILER_IR_CODE_H
