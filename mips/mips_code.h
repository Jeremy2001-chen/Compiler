//
// Created by chenjiyuan3 on 2021/11/7.
//

#ifndef COMPILER_MIPS_CODE_H
#define COMPILER_MIPS_CODE_H

#include "../ir/ir.h"
#include "../ir/ir_code.h"
#include <list>
#include <utility>
#include "../type.h"
#include <cstring>

using namespace std;

class MipsCode {
protected:
    MipsType type;
    string target, source[2], op;
public:
    virtual string toString() = 0;
};

/* add $s0 $s0 $s1 */
class MipsAdd : public MipsCode {
public:
    MipsAdd(string _op, string _tar, string _s0, string _s1) {
        target = std::move(_tar);
        source[0] = std::move(_s0);
        op = std::move(_op);
        if (op == "add" || op == "sub")
            op += "u";
        source[1] = std::move(_s1);
        type = MipsAddType;
    }

    string toString() override {
        return op + " " + target + " " + source[0] + " " + source[1];
    }
};

/* addi $s0 $s0 100 */
class MipsAddI : public MipsCode {
private:
    string imm;
public:
    MipsAddI(string _op, string _tar, string _s0, string _imm) {
        target = std::move(_tar);
        source[0] = std::move(_s0);
        source[1] = "";
        op = std::move(_op);
        if (op == "addi" || op == "subi")
            op += "u";
        imm = std::move(_imm);
        type = MipsAddIType;
    }

    string toString() override {
        return op + " " + target + " " + source[0] + " " + imm;
    }
};

/* mul $s0 $s1 */
class MipsMul : public MipsCode {
public:
    MipsMul(string _op, string _s0, string _s1) {
        target = "";
        op = std::move(_op);
        source[0] = std::move(_s0);
        source[1] = std::move(_s1);
        type = MipsMulType;
    }

    string toString() override {
        return op + " " + source[0] + " " + source[1];
    }
};

/* mfhi $s0 $s1 */
class MipsMF : public MipsCode {
public:
    MipsMF(string _op, string _s0) {
        op = std::move(_op);
        target = source[1] = "";
        source[0] = std::move(_s0);
        type = MipsMFType;
    }

    string toString() override {
        return op + " " + source[0];
    }
};

/* lw $s0 a($sp) */
class MipsLoad : public MipsCode {
private:
    string label; int labelType; // label can be an integer, labelType = 0 if integer else label
public:
    MipsLoad(string _op, string _tar, int offset, string _sor) {
        op = std::move(_op);
        target = std::move(_tar);
        label = to_string(offset);
        source[0] = std::move(_sor);
        source[1] = "";
        type = MipsLoadType;
        labelType = 0;
    }

    MipsLoad(string _op, string _tar, string _label, string _sor) {
        op = std::move(_op);
        target = std::move(_tar);
        label = std::move(_label);
        source[0] = std::move(_sor);
        source[1] = "";
        type = MipsLoadType;
        labelType = 1;
    }

    string toString() override {
        return op + " " + target + " " + label + "(" + source[0] + ")";
    }
};

/* sw $s0 a($sp) */
class MipsStore : public MipsCode {
private:
    string label; int labelType; // label can be an integer, labelType = 0 if integer else label
public:
    MipsStore(string _op, string _sor0, int offset, string _sor1) {
        op = std::move(_op);
        label = to_string(offset);
        source[0] = std::move(_sor0);
        source[1] = std::move(_sor1);
        target = "";
        type = MipsStoreType;
        labelType = 0;
    }

    MipsStore(string _op, string _sor0, string _label, string _sor1) {
        op = std::move(_op);
        label = std::move(_label);
        source[0] = std::move(_sor0);
        source[1] = std::move(_sor1);
        target = "";
        type = MipsStoreType;
        labelType = 1;
    }

    string toString() override {
        return op + " " + source[0] + " " + label + "(" + source[1] + ")";
    }
};

/* label: */
class MipsLabel : public MipsCode {
private:
    string label;
public:
    MipsLabel(string _label) {
        label = std::move(_label);
        target = source[0] = source[1] = "";
        type = MipsLabelType;
    }

    string toString() override {
        return label + ":";
    }
};

/* str1: .asciiz "hello" */
class MipsStringDef : public MipsCode {
private:
    string label, str;
public:
    MipsStringDef(string _label, string _str) {
        label = std::move(_label);
        str = std::move(_str);
        target = source[0] = source[1] = "";
        type = MIpsStringDefType;
    }

    string toString() override {
        return label + ": .asciiz \"" + str + "\"";
    }
};

/* g: .word 1 2 3 */
class MipsGlobalVarDef : public MipsCode {
private:
    string name, size;
    bool assign;
    vector<int> values;
public:
    MipsGlobalVarDef(string _name, int* value) {
        target = source[0] = source[1] = "";
        type = MipsGlobalVarDefType;
        name = std::move(_name);
        size = to_string(4);
        if (value == nullptr) {
            assign = false;
        } else {
            assign = true;
            values.push_back((*value));
        }
    }

    MipsGlobalVarDef(string _name, int _size, vector<int> *_values) {
        target = source[0] = source[1] = "";
        type = MipsGlobalVarDefType;
        name = std::move(_name);
        size = to_string(4 * _size);
        if (_values == nullptr) {
            assign = false;
        } else {
            assign = true;
            for (int i = 0; i < (int)(*_values).size(); i++)
                values.push_back((*_values)[i]);
        }
    }

    string toString() override {
        if (!assign)
            return name + ": .space " + size;
        else {
            string tmp = name + ": .word";
            for (auto v: values) {
                tmp += " " + to_string(v);
            }
            return tmp;
        }
    }
};

/* sw $s0 a($sp) */
class MipsBranch : public MipsCode {
private:
    string label;
public:
    MipsBranch(string _op, string _sc0, string _sc1, string _label) {
        op = std::move(_op);
        source[0] = std::move(_sc0);
        source[1] = std::move(_sc1);
        target = "";
        label = std::move(_label);
        type = MipsBranchType;
    }

    string toString() override {
        return op + " " + source[0] + " " + source[1] + " " + label;
    }
};

/* j label */
/* jal label */
class MipsJLabel: public MipsCode {
private:
    string label;
public:
    MipsJLabel(string _op, string _label) {
        target = source[0] = source[1] = "";
        label = _label;
        type = MipsJLabelType;
        op = std::move(_op);
        if (op == "jal")
            target = "$ra";
    }

    string toString() override {
        return op + " " + label;
    }
};

/* jr $ra */
class MipsJRegister: public MipsCode {
public:
    MipsJRegister(string _op, string _sc0) {
        target = source[1] = "";
        type = MipsJRegisterType;
        op = std::move(_op);
        source[0] = std::move(_sc0);
    }

    string toString() override {
        return op + " " + source[0];
    }
};

/*li $t0 100*/
class MipsLi: public MipsCode {
private:
    string imm;
public:
    MipsLi(string _op, string _sc0, string _imm) {
        target = source[1] = "";
        type = MipsLiType;
        op = std::move(_op);
        source[0] = std::move(_sc0);
        imm = std::move(_imm);
    }

    string toString() override {
        return op + " " + source[0] + " " + imm;
    }
};

/*la $t0 label*/
class MipsLa: public MipsCode {
private:
    string label;
public:
    MipsLa(string _op, string _sc0, string _label) {
        target = source[1] = "";
        type = MipsLaType;
        op = std::move(_op);
        source[0] = std::move(_sc0);
        label = std::move(_label);
    }

    string toString() override {
        return op + " " + source[0] + " " + label;
    }
};

/* syscall */
class MipsSyscall: public MipsCode {
public:
    MipsSyscall() {
        target = source[0] = source[1] = "";
        type = MipsSyscallType;
        op = "syscall";
    }

    string toString() override {
        return op;
    }
};

/* # xxxxxx */
class MipsNote: public MipsCode {
private:
    string note;
public:
    explicit MipsNote(string _note) {
        target = source[0] = source[1] = "";
        type = MipsNoteType;
        note = std::move(_note);
        op = "#";
    }

    string toString() {
        return op + " " + note;
    }
};

/* .data */
class MipsSegment: public MipsCode {
private:
    string label;
public:
    explicit MipsSegment(string _label) {
        label = std::move(_label);
        target = source[0] = source[1] = "";
        type = MipsSegmentType;
    }

    string toString() override {
        return "." + label;
    }
};

class MipsReg {
    int beginAddress = 0x10010000;
    int sp = 0x7fffeffc;
public:
    MipsReg() = default;
    int getSp() const {
        return sp;
    }
    void moveSp(int x) {
        sp += x;
    }
    int getBeginAdd() const {
        return beginAddress;
    }
    void moveBeginAdd(int x) {
        beginAddress += x;
    }
};

#endif //COMPILER_MIPS_CODE_H
