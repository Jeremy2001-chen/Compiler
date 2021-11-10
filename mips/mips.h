//
// Created by chenjiyuan3 on 2021/11/9.
//

#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#include <utility>

#include "mips_code.h"
#include "mips_table.h"
#include "mips_output.h"

class Mips{
private:
    vector <IrCode*>* irList;
    MipsTable *mipsTable;
    MipsOutput* mipsCode;
    int globalDeclEnd = 0;

public:
    Mips(IR _ir, MipsTable* _table, MipsOutput* _output) {
        irList = _ir.getIrList();
        mipsTable = _table;
        globalDeclEnd = _ir.getGlobalDeclEnd();
        mipsCode = _output;
        init_data();
        init_text();
    }

    void init_data() {
        mipsCode->push_back(new MipsSegment("data"));
        int line = 0;
        for (auto code: *irList) {
            if (code->getCodeType() == IrArrayDefineWithAssignType) {
                IrArrayDefineWithAssign* array = (IrArrayDefineWithAssign*)code;
                string name = IRNameTran(array->getName());
                mipsCode->push_back(new MipsGlobalVarDef(name, array->getSize(), array->getValues()));
                mipsTable->addGlobalTable(array->getName(), 4 * array->getSize());
            } else if (code->getCodeType() == IrArrayDefineWithOutAssignType) {
                IrArrayDefineWithOutAssign* array = (IrArrayDefineWithOutAssign*)code;
                string name = IRNameTran(array->getName());
                mipsCode->push_back(new MipsGlobalVarDef(name, array->getSize(), nullptr));
                mipsTable->addGlobalTable(array->getName(), 4 * array->getSize());
            } else if (code->getCodeType() == IrVarDefineWithAssignType) {
                IrVarDefineWithAssign* var = (IrVarDefineWithAssign*)code;
                int value = var->getValue();
                string name = IRNameTran(var->getName());
                mipsCode->push_back(new MipsGlobalVarDef(name, &value));
                mipsTable->addGlobalTable(var->getName(), 4);
            } else if (code->getCodeType() == IrVarDefineWithOutAssignType) {
                IrVarDefineWithOutAssign* var = (IrVarDefineWithOutAssign*)code;
                string name = IRNameTran(var->getName());
                mipsCode->push_back(new MipsGlobalVarDef(name, nullptr));
                mipsTable->addGlobalTable(var->getName(), 4);
            }
            line = line + 1;
            if (line > globalDeclEnd)
                break;
        }
        for (auto code: *irList) {
            if (code->getCodeType() == IrPrintStringType) {
                IrPrintString* str = (IrPrintString*)code;
                mipsTable->putString(str->getStr());
            }
        }
    }

    void init_text() {
        mipsCode-> push_back(new MipsSegment("text"));
        int count = 0, line = 0;
        for (int i = 0; i < (*irList).size(); ++ i) {
            IrCode* code = (*irList)[i];
            if (line <= globalDeclEnd) {
                if (code->getCodeType() == IrArrayDefineWithAssignType ||
                code->getCodeType() == IrArrayDefineWithOutAssignType ||
                code->getCodeType() == IrVarDefineWithAssignType ||
                code->getCodeType() == IrVarDefineWithOutAssignType)
                    continue;
                else
                    code->toMips();
            } else {
                if (code->getCodeType() == IrFunDefineType) {
                    code->toMips();
                    int j = i + 1, spMove = 0;
                    while (j < (*irList).size()) {
                        IrCode* codeN = (*irList)[j];
                        if (codeN->getCodeType() == IrFunEndType)
                            break;
                        spMove = spMove + (codeN -> defVar() << 2);
                        j = j + 1;
                    }
                    mipsOutput -> push_back(new MipsAddI("subi", "$sp", "$sp", to_string(spMove)));
                    mipsTable -> setStack(spMove);
                } else
                    code->toMips();
            }
        }
    }
};

#endif //COMPILER_MIPS_H
