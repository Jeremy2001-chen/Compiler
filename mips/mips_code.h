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

using namespace std;

class MipsCode;

extern MipsCode* mips;

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

class MipsTableItem {
private:
    string name;
    int layer, add;

public:
    MipsTableItem(string _name, int _layer, int _add) {
        name = std::move(_name);
        layer = _layer;
        add = _add;
    }
    string getName() {
        return name;
    }
    int getLayer() const {
        return layer;
    }
    int getAdd() const {
        return add;
    }
};

class MipsTable {
private:
    int layer = 0;
    vector<MipsTableItem> table;
    MipsReg* mipsReg;

public:
    explicit MipsTable(MipsReg* _mipsReg) {
        mipsReg = _mipsReg;
    }

    void setLayer(int det) {
        if (det < 0) {
            for (int i = (int)table.size() - 1; i >= 0; -- i) {
                if (table[i].getLayer() == layer)
                    table.pop_back();
            }
        }
        layer += det;
    }

    /* return the address of var*/
    int addTable(const string& name) {
        for (int i = (int)table.size() - 1; i >= 0; -- i) {
            if (table[i].getLayer() < layer)
                break;
            if (table[i].getName() == name) {
                return table[i].getAdd();
            }
        }
        int add = mips -> getSp();
        table.push_back(new MipsTableItem(name, layer, ))
    }
};

#endif //COMPILER_MIPS_CODE_H
