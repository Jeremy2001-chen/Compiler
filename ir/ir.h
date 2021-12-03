//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_H
#define COMPILER_IR_H

#include <cstring>
#include "ir_code.h"
#include "ir_fun.h"
#include "../list/mylist.h"

class IR {
private:
    vector <IrCode*> irList; //origin IR code
    int globalDeclEnd;
public:
    IR() = default;
    string toString() {
        string ret;
        for (auto ir: irList) {
            ret += ir->toString() + "\n";
        }
        return ret;
    }

    void add(IrCode* irCode) {
#ifdef DEBUG
        cout << "New line~~~~~~~: " << irCode->toString() << endl;
#endif
        irList.push_back(irCode);
    }

    void setGlobalDeclEnd() {
        globalDeclEnd = (int)irList.size();
    }

    void setGlobalDeclEnd(int x) {
        globalDeclEnd = x;
    }

    int getGlobalDeclEnd() const {
        return globalDeclEnd;
    }

    vector <IrCode*>* getIrList() {
        return &irList;
    }

    int getIrSize() {
        return (int)irList.size();
    }
};

map <string, int> globalNameCount; //for ssa to rename
class IrNew {
private:
    vector <IrCode*> irDecl;
    vector <IrFun*> irFun;
public:
    IrNew(IR* ir) {
        globalNameCount.clear();
        vector <IrCode*>* list = ir -> getIrList();
        int decl = ir -> getGlobalDeclEnd();
        vector<IrCode*> *funCode = nullptr;
        for (int i = 0; i < decl; ++ i)
            irDecl.push_back((*list)[i]);
        for (int i = decl; i < (*list).size(); ++ i) {
            if (((*list)[i]) -> getCodeType() == IrFunDefineType) {
                if (funCode != nullptr) {
                    IrFun* fun = new IrFun(funCode);
                    irFun.push_back(fun);
                    funCode = new vector<IrCode*>();
                } else
                    funCode = new vector<IrCode*>();
            }
            (*funCode).push_back((*list)[i]);
        }
        if (funCode != nullptr) {
            IrFun* fun = new IrFun(funCode);
            irFun.push_back(fun);
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
};
#endif //COMPILER_IR_H
