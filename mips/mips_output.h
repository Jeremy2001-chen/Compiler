//
// Created by chenjiyuan3 on 2021/11/9.
//

#ifndef COMPILER_MIPS_OUTPUT_H
#define COMPILER_MIPS_OUTPUT_H

#include "mips_code.h"

class MipsOutput {
private:
    vector<MipsCode*> mipsCode;
public:
    MipsOutput() = default;

    void push_back(MipsCode* mipsCode1) {
        mipsCode.push_back(mipsCode1);
    }

    string toString() {
        string str;
        for (auto code: mipsCode)
            str += code->toString() + "\n";
        return str;
    }
};
#endif //COMPILER_MIPS_OUTPUT_H
