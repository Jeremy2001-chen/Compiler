//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_H
#define COMPILER_IR_H

#include <list>
#include <cstring>
#include "ir_code.h"

class IR {
private:
    list <IrCode*> irList;
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
        cout << "New line~~~~~~~: " << irCode->toString() << endl;
        irList.push_back(irCode);
    }
};

#endif //COMPILER_IR_H
