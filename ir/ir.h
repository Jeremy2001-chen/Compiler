//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_H
#define COMPILER_IR_H

#include <cstring>
#include "ir_code.h"
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

#endif //COMPILER_IR_H
