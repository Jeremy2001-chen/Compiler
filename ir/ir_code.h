//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_CODE_H
#define COMPILER_IR_CODE_H

#include <cstring>
#include <utility>
using namespace std;

class IrCode {
protected:
    int codeType;
public:
    virtual string toString() = 0;
    virtual void toMips() = 0;
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

    }
};

class IrFunDefine: public IrCode {
private:
    string type, name;
public:
    IrFunDefine(string _type, string _name) {
        type = std::move(_type);
        name = std::move(_name);
        codeType = IrFunDefineType;
    }

    string toString() override {
        return type + " " + name + "()";
    }

    void toMips() override {

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

    }
};

class IrPushArray: public IrCode {
private:
    string name, offset;
public:
    IrPushArray(string _name, string _offset) {
        name = std::move(_name);
        offset = std::move(_offset);
        codeType = IrPushArrayType;
    }

    string toString() override {
        return "push array " + name + "[" + offset + "]";
    }

    void toMips() override {

    }
};

class IrCallFunction: public IrCode {
private:
    string name;
public:
    explicit IrCallFunction(string _name) {
        name = std::move(_name);
        codeType = IrCallFunctionType;
    }

    string toString() override {
        return "call " + name;
    }

    void toMips() override {

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
        return "ret " + source;
    }

    void toMips() override {

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

    void toMips() override {

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

    void toMips() override {

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

    void toMips() override {

    }
};

class IrBranchStmt: public IrCode {
private:
    string type, name;
public:
    IrBranchStmt(string _type, string _name) {
        type = std::move(_type);
        name = std::move(_name);
        codeType = IrBranchStmtType;
    }

    string toString() override {
        return type + " " + name;
    }

    void toMips() override {

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

    void toMips() override {

    }
};

class IrArrayDefineWithAssign: public IrCode {
private:
    bool isConst;
    string name;
    vector<int>* value;
public:
    IrArrayDefineWithAssign(bool _is, string _na, vector<int>* var) {
        isConst = _is;
        name = std::move(_na);
        value = var;
        codeType = IrArrayDefineWithAssignType;
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        tmp = tmp + "arr int " + name + "[" + to_string((*value).size()) + "] =";
        for (int i : *value)
            tmp += " " + to_string(i);
        return tmp;
    }

    void toMips() override {

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

    void toMips() override {

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

    }

    string getStr() {
        return str;
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

    }
};
#endif //COMPILER_IR_CODE_H
