//
// Created by chenjiyuan3 on 2021/11/6.
//

#ifndef COMPILER_IR_TABLE_H
#define COMPILER_IR_TABLE_H
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "../ir/ir.h"
#include "../ir/ir_code.h"
#include "../ir/ir_table.h"


using namespace std;

class IrTable {
private:
    string name, irName;
    bool isGlobal;
    int layer;
public:
    IrTable(string _name, string new_name, bool global, int _layer) {
        name = std::move(_name);
        irName = std::move(new_name);
        isGlobal = global;
        layer = _layer;
    }

    int getLayer() const {
        return layer;
    }

    string getName() const {
        return name;
    }

    string getIrName() const {
        return irName;
    }
};

class IrTableList {
private:
    vector<IrTable> tableList;
    vector<string> loopStack;
    vector<string> temRegister;
    map<string, int> nameCount;
    int layer = 0;
    int temNumber = 1;
    int branchNumber = 1;
    int loopNumber = 1;
public:
    IrTableList() = default;
    void intoBlock() {
        layer += 1;
    }

    void setBlock(int det) {
        layer += det;
    }

    void popBlock() {
        int tot = (int)tableList.size();
        for (int i = tot - 1; i >= 0; -- i)
            if (tableList[i].getLayer() == layer)
                tableList.pop_back();
            else
                break;
        layer -= 1;
    }


    string addVar(const string& _name) {
        string irName;
        auto it = nameCount.find(_name);
        if (it == nameCount.end()) {
            nameCount[_name] = 1;
            irName = (layer == 0 ? "@" : "%") + _name;
        } else {
            irName = (layer == 0 ? "@" : "%") + _name + "_" + to_string(nameCount[_name]);
            nameCount[_name] ++;
        }
        bool isGlobal = (layer == 0);
        tableList.emplace_back(_name, irName, isGlobal, layer);
        return irName;
    }

    string getIrName(const string& _name) {
        int tot = (int)tableList.size();
        for (int i = tot - 1; i >= 0; -- i)
            if (tableList[i].getName() == _name)
                return tableList[i].getIrName();
        return {};
    }

    string allocTem() {
        addTemNumber();
        string irName = (layer == 0 ? "@" : "%") + to_string(temNumber);
        temRegister.push_back(irName);
        return irName;
    }

    string getTopTemIrName() {
        return temRegister[(int)temRegister.size() - 1];
    }

    int getTemNumber() {
        return temNumber;
    }

    void addTemNumber() {
        temNumber += 1;
    }

    int getBranchNum() {
        return branchNumber;
    }

    string allocBranch() {
        string branchName = "_branch__" + to_string(branchNumber) + "_chenjiyuan";
        branchNumber += 1;
        return branchName;
    }

    int getLoopNum() {
        return loopNumber;
    }

    string allocLoop() {
        string loopName = "_loop__" + to_string(loopNumber) + "_chenjiyuan";
        loopStack.push_back(loopName);
        loopNumber += 1;
        return loopName;
    }

    void popLoop() {
        loopStack.pop_back();
    }

    string getTopLoop() {
        return loopStack[(int)loopStack.size() - 1];
    }

} irTableList_1;


#endif //COMPILER_IR_TABLE_H
