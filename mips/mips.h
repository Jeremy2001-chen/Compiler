//
// Created by chenjiyuan3 on 2021/11/9.
//

#ifndef COMPILER_MIPS_H
#define COMPILER_MIPS_H

#include <utility>

#include "mips_code.h"
#include "mips_table.h"

class Mips{
private:
    list <IrCode*>* irList;
    MipsTable mipsTable;
    map<int, string> strLineToLabel;
    unsigned int sp, data;
    vector<MipsCode*> mipsCode;

public:
    Mips(IR _ir) {
        irList = _ir.getIrList();
        data = 0x10010000;
        sp = 0x7fffeffc;
        init_data();
        init_text();
    }

    string IRNameTran(const string& str) {
        return str.substr(1);
    }

    void init_data() {
        mipsCode.push_back(new MipsSegment("data"));
        int count = 0, line = 0;
        for (auto code: *irList) {
            if (code->getCodeType() == IrArrayDefineWithAssignType) {
                IrArrayDefineWithAssign* array = (IrArrayDefineWithAssign*)code;
                string name = IRNameTran(array->getName());
                mipsCode.push_back(new MipsGlobalVarDef(name, array->getSize(), array->getValues()));
                mipsTable.addTable(array->getName(), data);
                data += 4 * array->getSize();
            } else if (code->getCodeType() == IrArrayDefineWithOutAssignType) {
                IrArrayDefineWithOutAssign* array = (IrArrayDefineWithOutAssign*)code;
                string name = IRNameTran(array->getName());
                mipsCode.push_back(new MipsGlobalVarDef(name, array->getSize(), nullptr));
                mipsTable.addTable(array->getName(), data);
                data += 4 * array->getSize();
            } else if (code->getCodeType() == IrVarDefineWithAssignType) {
                IrVarDefineWithAssign* var = (IrVarDefineWithAssign*)code;
                int value = var->getValue();
                string name = IRNameTran(var->getName());
                mipsCode.push_back(new MipsGlobalVarDef(name, &value));
                mipsTable.addTable(var->getName(), data);
                data += 4;
            } else if (code->getCodeType() == IrVarDefineWithOutAssignType) {
                IrVarDefineWithOutAssign* var = (IrVarDefineWithOutAssign*)code;
                string name = IRNameTran(var->getName());
                mipsCode.push_back(new MipsGlobalVarDef(name, nullptr));
                mipsTable.addTable(var->getName(), data);
                data += 4;
            } else
                break;
        }
        for (auto code: *irList) {
            if (code->getCodeType() == IrPrintStringType) {
                IrPrintString* str = (IrPrintString*)code;
                count = count + 1;
                string label = "__str_chenjiyuan_" + to_string(count);
                strLineToLabel[line] = label;
                mipsCode.push_back(new MipsStringDef(label, str->getStr()));
            }
            line = line + 1;
        }
    }

    void init_text() {
        mipsCode.push_back(new MipsSegment("text"));
    }

    string toString() {
        string str;
        for (auto code: mipsCode)
            str += code->toString() + "\n";
        return str;
    }
};

#endif //COMPILER_MIPS_H
