//
// Created by chenjiyuan3 on 2021/12/4.
//

#ifndef COMPILER_IR_NEW_H
#define COMPILER_IR_NEW_H

#include "ir_fun.h"
#include "../mips/mips_code.h"

extern MipsTable *mipsTable;
extern MipsOutput *mipsOutput;

map <string, int> globalNameCount; //for ssa to rename
class IrNew {
private:
    vector <IrCode*> irDecl;
    vector <IrFun*> irFun;
    vector <IrCode*>* origin;
public:
    IrNew(IR* ir) {
        globalNameCount.clear();
        vector <IrCode*>* list = ir -> getIrList();
        origin = list;
        int decl = ir -> getGlobalDeclEnd();
        vector<IrCode*> *funCode = nullptr;
        for (int i = 0; i < decl; ++ i)
            irDecl.push_back((*list)[i]);
//        init_data();
//        mipsOutput -> push_back(new MipsSegment("text"));
        varToRegister = new map<string, string>();
        for (int i = decl; i < (*list).size(); ++ i) {
            if (((*list)[i]) -> getCodeType() == IrFunDefineType) {
                if (funCode != nullptr) {
                    IrFun* fun = new IrFun(funCode);
                    irFun.push_back(fun);
//                    fun -> toMips();
                    funCode = new vector<IrCode*>();
                } else
                    funCode = new vector<IrCode*>();
            }
            (*funCode).push_back((*list)[i]);
        }
        if (funCode != nullptr) {
            IrFun* fun = new IrFun(funCode);
            irFun.push_back(fun);
//            fun -> toMips();
        }
    }

    string toString() {
        string ret;
        for (auto a: irFun) {
            ret += a -> toString() + "\n";
        }
        return ret;
    }

    vector<IrCode*>* toIR() {
        //cout << toString() << endl;
        vector<IrCode*>* newIR = new vector<IrCode*>();
        for (auto code: irDecl)
            newIR -> push_back(code);
        for (auto fun: irFun) {
            vector<IrCode*>* codes = fun -> toIR();
            for (auto code: *codes)
                newIR -> push_back(code);
        }
        return newIR;
    }

    void analysis() {
        for (auto fun: irFun) {
            fun -> dataAnalyse();
        }
    }

    void toMips() {
        init_data();
        mipsOutput -> push_back(new MipsSegment("text"));
        for (auto fun: irFun) {
            fun -> toMips();
        }
    }

    void init_data() {
        mipsOutput->push_back(new MipsSegment("data"));
        int count = 0;
        for (auto code: irDecl) {
            if (code->getCodeType() == IrArrayDefineWithAssignType) {
                IrArrayDefineWithAssign* array = (IrArrayDefineWithAssign*)code;
                string name = IRNameTran(array->getName());
                mipsOutput->push_back(new MipsGlobalVarDef(name, array->getSize(), array->getValues()));
                mipsTable->addGlobalTable(array->getName(), 4 * array->getSize());
            } else if (code->getCodeType() == IrArrayDefineWithOutAssignType) {
                IrArrayDefineWithOutAssign* array = (IrArrayDefineWithOutAssign*)code;
                string name = IRNameTran(array->getName());
                mipsOutput->push_back(new MipsGlobalVarDef(name, array->getSize(), nullptr));
                mipsTable->addGlobalTable(array->getName(), 4 * array->getSize());
            } else if (code->getCodeType() == IrVarDefineWithAssignType) {
                IrVarDefineWithAssign* var = (IrVarDefineWithAssign*)code;
                int value = var->getValue();
                string name = IRNameTran(var->getName());
                mipsOutput->push_back(new MipsGlobalVarDef(name, &value));
                mipsTable->addGlobalTable(var->getName(), 4);
            } else if (code->getCodeType() == IrVarDefineWithOutAssignType) {
                IrVarDefineWithOutAssign* var = (IrVarDefineWithOutAssign*)code;
                string name = IRNameTran(var->getName());
                mipsOutput->push_back(new MipsGlobalVarDef(name, nullptr));
                mipsTable->addGlobalTable(var->getName(), 4);
            }
        }
        for (auto code: irDecl) {
            if (code->getCodeType() == IrNumberAssignType) {
                IrNumberAssign* var = (IrNumberAssign*)code;
                string name = IRNameTran(var->getTarget());
                if (name[0] >= '0' && name[0] <= '9') {
                    count ++;
                    mipsTable->addGlobalTable(var->getTarget(), 4);
                }
            } else if (code->getCodeType() == IrBinaryOpType) {
                IrBinaryOp* var = (IrBinaryOp*)code;
                string name = IRNameTran(var->getTarget());
                if (name[0] >= '0' && name[0] <= '9') {
                    count ++;
                    mipsTable->addGlobalTable(var->getTarget(), 4);
                }
            } else if (code->getCodeType() == IrUnaryOpType) {
                IrUnaryOp* var = (IrUnaryOp*)code;
                string name = IRNameTran(var->getTarget());
                if (name[0] >= '0' && name[0] <= '9') {
                    count ++;
                    mipsTable->addGlobalTable(var->getTarget(), 4);
                }
            } else if (code->getCodeType() == IrArrayGetType) {
                IrArrayGet* var = (IrArrayGet*)code;
                string name = IRNameTran(var->getTarget());
                if (name[0] >= '0' && name[0] <= '9') {
                    count ++;
                    mipsTable->addGlobalTable(var->getTarget(), 4);
                }
            }
        }
        mipsOutput -> push_back(new MipsGlobalVarDef("__global_var_all", count, nullptr));
        for (auto code: *origin) {
            if (code->getCodeType() == IrPrintStringType) {
                IrPrintString* str = (IrPrintString*)code;
                mipsTable -> putString(str->getStr());
            }
        }
        mipsTable->setDataAlign();
    }

    void init_text() {
        mipsOutput -> push_back(new MipsSegment("text"));
        for (auto c: irFun) {
            c -> toMips();
        }
    }

    vector<IrFun*>* getIrFun() {
        return &irFun;
    }
};

auto* varToInt = new map<string, int>();
auto* varToVar = new map<string, string>();

inline bool isOtherVar(string name) {
    return varToVar -> find(name) != varToVar -> end();
}

inline string getOtherVar(string name) {
    return (*varToVar)[name];
}

inline bool isInt(string name) {
    if (isOtherVar(name))
        return varToInt -> find(getOtherVar(name)) != varToInt -> end();
    return varToInt -> find(name) != varToInt -> end();
}

inline int getInt(string name) {
    if (isOtherVar(name))
        return (*varToInt)[getOtherVar(name)];
    return (*varToInt)[name];
}

inline void setVarToVar(string source, string target) {
    if (source[0] != '%')
        return ;
    if (isOtherVar(target))
        (*varToVar)[source] = getOtherVar(target);
    else
        (*varToVar)[source] = target;
}

inline void setNumber(string source, int number) {
    if (source[0] != '%')
        return ;
    (*varToInt)[source] = number;
}

void constSpread(IrNew* irNew) {
    (*varToInt)["%0"] = 0;
    auto* funList = irNew->getIrFun();
    for (auto c: *funList) {
        auto* block = c -> getFirstBlock();
        while (block != nullptr) {
            IrBlock* block1 = block -> getBlock();
            MyList* start = block1 -> getStartCode();
            while (start != nullptr) {
                IrCode* code = start -> getCode();
                switch (code -> getCodeType()) {
                    case IrBinaryOpType:
                    {
                        IrBinaryOp* binary = (IrBinaryOp*)code;
                        string ta = binary -> getTarget(), sign = binary -> getSign(), s0 = binary -> getSource(0), s1 = binary -> getSource(1);
                        if (isInt(s0) && isInt(s1)) {
                            int number = 0;
                            if (sign == "+")
                                number = getInt(s0) + getInt(s1);
                            else if (sign == "-")
                                number = getInt(s0) - getInt(s1);
                            else if (sign == "*")
                                number = getInt(s0) * getInt(s1);
                            else if (sign == "/")
                                number = getInt(s0) / getInt(s1);
                            else if (sign == "%")
                                number = getInt(s0) % getInt(s1);
                            else if (sign == "<")
                                number = (getInt(s0) < getInt(s1));
                            else if (sign == ">")
                                number = (getInt(s0) > getInt(s1));
                            else if (sign == ">=")
                                number = (getInt(s0) >= getInt(s1));
                            else if (sign == "<=")
                                number = (getInt(s0) <= getInt(s1));
                            else if (sign == "==")
                                number = (getInt(s0) == getInt(s1));
                            else if (sign == "!=")
                                number = (getInt(s0) != getInt(s1));
                            IrNumberAssign* assign = new IrNumberAssign(ta, to_string(number));
                            MyList* myList = new MyList(assign);
                            block1 -> replace(start, myList);
                            start = myList;
                            setNumber(ta, number);
                        } else if (isInt(s0)) {
                            int v0 = getInt(s0);
                            if (v0 == 0) {
                                if (sign == "+") {
                                    IrUnaryOp* unaryOp = new IrUnaryOp(ta, "+", s1);
                                    if (isOtherVar(s1))
                                        unaryOp -> setSource(0, getOtherVar(s1));
                                    MyList* myList = new MyList(unaryOp);
                                    block1 -> replace(start, myList);
                                    start = myList;
                                    setVarToVar(ta, s1);
                                } else if (sign == "*") {
                                    IrNumberAssign* assign = new IrNumberAssign(ta, "0");
                                    MyList* myList = new MyList(assign);
                                    block1 -> replace(start, myList);
                                    start = myList;
                                    setNumber(ta, 0);
                                }
                            } else if (v0 == 1 && sign == "*") {
                                IrUnaryOp* unaryOp = new IrUnaryOp(ta, "+", s1);
                                if (isOtherVar(s1))
                                    unaryOp -> setSource(0, getOtherVar(s1));
                                MyList* myList = new MyList(unaryOp);
                                block1 -> replace(start, myList);
                                start = myList;
                                setVarToVar(ta, s1);
                            } else if (v0 == 2 && sign == "*") {
                                IrBinaryOp* binaryOp = new IrBinaryOp(ta, s1, "+", s1);
                                if (isOtherVar(s1)) {
                                    binaryOp -> setSource(0, getOtherVar(s1));
                                    binaryOp -> setSource(1, getOtherVar(s1));
                                }
                                MyList* myList = new MyList(binaryOp);
                                block1 -> replace(start, myList);
                                start = myList;
                            } else if (v0 == -1 && sign == "*") {
                                IrUnaryOp* unaryOp = new IrUnaryOp(ta, "-", s1);
                                if (isOtherVar(s1))
                                    unaryOp -> setSource(0, getOtherVar(s1));
                                MyList* myList = new MyList(unaryOp);
                                block1 -> replace(start, myList);
                                start = myList;
                            } else if (sign == "+") {
                                binary -> setSource(0, s1);
                                binary -> setSource(1, to_string(v0));
                                if (isOtherVar(s1))
                                    binary -> setSource(0, getOtherVar(s1));
                            } else { //todo
                                if (isOtherVar(s0))
                                    binary -> setSource(0, getOtherVar(s0));
                                if (isOtherVar(s1))
                                    binary -> setSource(1, getOtherVar(s1));
                            }
                        } else if (isInt(s1)) {
                            int v1 = getInt(s1);
                            if (v1 == 0) {
                                if (sign == "+") {
                                    IrUnaryOp* unaryOp = new IrUnaryOp(ta, "+", s0);
                                    if (isOtherVar(s0))
                                        unaryOp -> setSource(0, getOtherVar(s0));
                                    MyList* myList = new MyList(unaryOp);
                                    block1 -> replace(start, myList);
                                    start = myList;
                                    setVarToVar(ta, s0);
                                } else if (sign == "*") {
                                    IrNumberAssign* assign = new IrNumberAssign(ta, "0");
                                    MyList* myList = new MyList(assign);
                                    block1 -> replace(start, myList);
                                    start = myList;
                                    setNumber(ta, 0);
                                }
                            } else if (v1 == 1 && (sign == "*" || sign == "/")) {
                                IrUnaryOp* unaryOp = new IrUnaryOp(ta, "+", s0);
                                if (isOtherVar(s0))
                                    unaryOp -> setSource(0, getOtherVar(s0));
                                MyList* myList = new MyList(unaryOp);
                                block1 -> replace(start, myList);
                                start = myList;
                                setVarToVar(ta, s0);
                            } else if (v1 == 2 && sign == "*") {
                                IrBinaryOp* binaryOp = new IrBinaryOp(ta, s0, "+", s0);
                                if (isOtherVar(s1)) {
                                    binaryOp -> setSource(0, getOtherVar(s0));
                                    binaryOp -> setSource(1, getOtherVar(s0));
                                }
                                MyList* myList = new MyList(binaryOp);
                                block1 -> replace(start, myList);
                                start = myList;
                            } else if (v1 == -1 && sign == "*") {
                                IrUnaryOp* unaryOp = new IrUnaryOp(ta, "-", s0);
                                if (isOtherVar(s0))
                                    unaryOp -> setSource(0, getOtherVar(s0));
                                MyList* myList = new MyList(unaryOp);
                                block1 -> replace(start, myList);
                                start = myList;
                            } else if (sign == "+" || sign == "-") {
                                if (isOtherVar(s0))
                                    binary -> setSource(0, getOtherVar(s0));
                                binary -> setSource(1, to_string(v1));
                            } else { //todo
                                if (isOtherVar(s0))
                                    binary -> setSource(0, getOtherVar(s0));
                                if (isOtherVar(s1))
                                    binary -> setSource(1, getOtherVar(s1));
                            }
                        } else {
                            if (isOtherVar(s0))
                                binary -> setSource(0, getOtherVar(s0));
                            if (isOtherVar(s1))
                                binary -> setSource(1, getOtherVar(s1));
                        }
                        break;
                    }
                    case IrUnaryOpType:
                    {
                        IrUnaryOp* unaryOp = (IrUnaryOp*)code;
                        string ta = unaryOp -> getTarget(), sign = unaryOp -> getSign(), s0 = unaryOp -> getSource(0);
                        if (isInt(s0)) {
                            int number = 0;
                            if (sign == "+")
                                number = getInt(s0);
                            else if (sign == "-")
                                number = -getInt(s0);
                            else if (sign == "!")
                                number = !getInt(s0);
                            IrNumberAssign* assign = new IrNumberAssign(ta, to_string(number));
                            MyList* myList = new MyList(assign);
                            block1 -> replace(start, myList);
                            start = myList;
                            setNumber(ta, number);
                        } else if (sign == "+") {
                            if (isOtherVar(s0))
                                unaryOp -> setSource(0, getOtherVar(s0));
                            setVarToVar(ta, s0);
                        } else {
                            if (isOtherVar(s0))
                                unaryOp -> setSource(0, getOtherVar(s0));
                        }
                        break;
                    }
                    case IrNumberAssignType:
                    {
                        IrNumberAssign* assign = (IrNumberAssign*)code;
                        string ta = assign -> getTarget();
                        int number = assign -> getNumber();
                        setNumber(ta, number);
                        break;
                    }
                    case IrVarDefineWithAssignType:
                    {
                        IrVarDefineWithAssign* assign = (IrVarDefineWithAssign*)code;
                        string ta = assign -> getTarget();
                        int number = assign -> getValue();
                        setNumber(ta, number);
                        break;
                    }
                    case IrBranchStmtType:
                    {
                        IrBranchStmt* branch = (IrBranchStmt*)code;
                        string type = branch -> getType(), label = branch -> getLabel();
                        string s0 = branch -> getSource(0), s1 = branch -> getSource(1);
                        if (type == "beq") {
                            if (isInt(s0) && isInt(s1)) {
                                int v0 = getInt(s0), v1 = getInt(s1);
                                if (v0 == v1) {
                                    IrGotoStmt* gotoStmt = new IrGotoStmt(label);
                                    MyList* myList = new MyList(gotoStmt);
                                    block1 -> addReplace(start, myList);
                                } else {
                                    IrNop* nop = new IrNop();
                                    MyList* myList = new MyList(nop);
                                    block1 -> addReplace(start, myList);
                                }
                            }
                        } else if (type == "bne") {
                            if (isInt(s0) && isInt(s1)) {
                                int v0 = getInt(s0), v1 = getInt(s1);
                                if (v0 != v1) {
                                    IrGotoStmt* gotoStmt = new IrGotoStmt(label);
                                    MyList* myList = new MyList(gotoStmt);
                                    block1 -> addReplace(start, myList);
                                } else {
                                    IrNop* nop = new IrNop();
                                    MyList* myList = new MyList(nop);
                                    block1 -> addReplace(start, myList);
                                }
                            }
                        } else if (type == "bgt") {
                            if (isInt(s0) && isInt(s1)) {
                                int v0 = getInt(s0), v1 = getInt(s1);
                                if (v0 > v1) {
                                    IrGotoStmt* gotoStmt = new IrGotoStmt(label);
                                    MyList* myList = new MyList(gotoStmt);
                                    block1 -> addReplace(start, myList);
                                } else {
                                    IrNop* nop = new IrNop();
                                    MyList* myList = new MyList(nop);
                                    block1 -> addReplace(start, myList);
                                }
                            }
                        }
                        break;
                    }
                    default:
                    {
                        for (int i = 0; i < 3; ++ i) {
                            string source = code -> getSource(i);
                            if (isInt(source))
                                code -> setSource(i, to_string(getInt(source)));
                            else if (isOtherVar(source))
                                code -> setSource(i, getOtherVar(source));
                        }
                        break;
                    }
                }
                start = start -> getNext();
            }
            block = block -> getNext();
        }
    }
}

void removeAddZero(IrNew* irNew) {
    auto* funList = irNew->getIrFun();
    for (auto c: *funList) {
        auto* block = c -> getFirstBlock();
        while (block != nullptr) {
            IrBlock* block1 = block -> getBlock();
            MyList* start = block1 -> getStartCode();
            while (start != nullptr) {
                IrCode* code = start -> getCode();
                if (code -> getCodeType() == IrBinaryOpType) {
                    IrBinaryOp* binary = (IrBinaryOp*)code;
                    string sign = binary -> getSign();
                    if (binary -> getSource(1) == "%0") {
                        if (sign == "+" || sign == "-") {
                            IrUnaryOp* unaryOp = new IrUnaryOp(binary -> getTarget(), "+", binary -> getSource(0));
                            MyList* myList = new MyList(unaryOp);
                            block1 -> replace(start, myList);
                            start = myList;
                        } else if (sign == "*") {
                            IrNumberAssign* assign = new IrNumberAssign(binary -> getTarget(), "0");
                            MyList* myList = new MyList(assign);
                            block1 -> replace(start, myList);
                            start = myList;
                        }
                    }
                }
                start = start -> getNext();
            }
            block = block -> getNext();
        }
    }
}

#endif //COMPILER_IR_NEW_H

//todo
// now is wrong for the def & use is not true!!!!