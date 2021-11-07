//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_CODE_H
#define COMPILER_IR_CODE_H

#include <cstring>
using namespace std;

class IrCode {
public:
    virtual string toString() = 0;
    virtual void toMips() = 0;
};

class IrBinaryOp: public IrCode {
private:
    string target, source[2];
    string sign;
public:
    IrBinaryOp(string _t, string _sc0, string _sign, string _sc1) {
        target = _t;
        source[0] = _sc0;
        source[1] = _sc1;
        sign = _sign;
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
    }

    string toString() override {
        return target + " = " + source[0] + " " + sign + " " + source[1];
    }

    void toMips() override {

    }
};

class IrLabelLine: public IrCode {
private:
    string label;
public:
    IrLabelLine(string _label) {
        label = std::move(_label);
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
        type = _type;
        name = _name;
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
        type = _type;
        name = _name;
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
    IrPushVariable(string _name) {
        name = _name;
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
        name = _name;
        offset = _offset;
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
    IrCallFunction(string _name) {
        name = _name;
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
    }

    IrReturnStmt(string _sc) {
        source = _sc;
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
    IrReturnValStmt(string _tar) {
        target = _tar;
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
        name = _na;
        value = _va;
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
        name = _na;
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
        source[0] = sc0;
        source[1] = sc1;
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
    IrGotoStmt(string _label) {
        label = _label;
    }

    string toString() override {
        return "goto " + label;
    }

    void toMips() override {

    }
};

class IrArrayDefine: public IrCode {
private:
    bool isConst;
    string name, offset;
public:
    IrArrayDefine(bool _is, string _na, string _off) {
        isConst = _is;
        name = std::move(_na);
        offset = std::move(_off);
    }

    string toString() override {
        string tmp = (isConst ? "const " : "");
        return tmp + "arr int " + name + "[" + offset + "]";
    }

    void toMips() override {

    }
};

class IrArrayAssign: public IrCode {
private:
    string target, offset, source;
public:
    IrArrayAssign(string _ta, string _off, string _sc) {
        target = _ta;
        offset = _off;
        source = _sc;
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
    IrReadInteger(string _ta) {
        target = _ta;
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
    IrPrintInteger(string _sc) {
        source = _sc;
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
    IrPrintString(string _str) {
        str = _str;
    }

    string toString() override {
        return "printInt \"" + str + "\"";
    }

    void toMips() override {

    }
};

class IrNumberAssign: public IrCode {
private:
    string target, number;
public:
    IrNumberAssign(string _ta, string _num) {
        target = _ta;
        number = _num;
    }

    string toString() override {
        return target + " = " + number;
    }

    void toMips() override {

    }
};
#endif //COMPILER_IR_CODE_H
