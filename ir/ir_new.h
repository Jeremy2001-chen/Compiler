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

void removeAddZero(IrNew* irNew) {
    auto* funList = irNew->getIrFun();
    for (auto c: *funList) {
        auto* block = c -> getFirstBlock();
//        cout << "TTTT: " << endl;
        while (block != nullptr) {
            IrBlock* block1 = block -> getBlock();
            MyList* start = block1 -> getStartCode();
//            cout << "GGGG: " << endl;
            while (start != nullptr) {
                IrCode* code = start -> getCode();
                if (code -> getCodeType() == IrBinaryOpType) {
                    IrBinaryOp* binary = (IrBinaryOp*)code;
                    string sign = binary -> getSign();
//                    cout << "&&&&&\n" << code -> toString() << endl;
                    if (binary -> getSource(1) == "%0" && (sign == "+" || sign == "-")) {
//                        cout << "!!!!!!\n" << code -> toString() << endl;
                        IrUnaryOp* unaryOp = new IrUnaryOp(binary -> getTarget(), "+", binary -> getSource(0));
                        MyList* myList = new MyList(unaryOp);
                        block1 -> replace(start, myList);
                        start = myList;
                    }
                }
                start = start -> getNext();
            }
            block = block -> getNext();
        }
    }
}

#endif //COMPILER_IR_NEW_H
