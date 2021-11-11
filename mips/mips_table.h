//
// Created by chenjiyuan3 on 2021/11/9.
//

#ifndef COMPILER_MIPS_TABLE_H
#define COMPILER_MIPS_TABLE_H

#include "mips_output.h"

extern MipsOutput* mipsOutput;

string IRNameTran(const string& str) {
    return str.substr(1);
}

class MipsTableItem {
private:
    string name;
    int layer;
    int add;
    bool temporary;

public:
    MipsTableItem(string _name, int _layer, int _add, bool _tem) {
        name = std::move(_name);
        layer = _layer;
        add = _add;
        temporary = _tem;
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
    bool getTemporary() const {
        return temporary;
    }
};

class MipsTable {
private:
    int layer = 0;
    int sp = 0x7ffffff0, data = 0x10010000;
    vector<MipsTableItem> table;
    stack<int> paraCnt;
    int pushCnt = 0;
    map<string, string> stringToLabel;
    int strCount = 0;

public:
    MipsTable() {
        paraCnt.push(0);
    }

    void setDataAlign() {
        data = (data % 4 == 0 ? data : data + 4 - data % 4);
    }

    void putString(const string& str) {
        auto it = stringToLabel.find(str);
        if (it == stringToLabel.end()) {
            strCount ++;
            string label = "__str_chenjiyuan_" + to_string(strCount);
            stringToLabel[str] = label;
            mipsOutput -> push_back(new MipsStringDef(label, str));
            data += (int)str.size() + 1;
        }
    }

    string getLabel(const string& str) {
        auto it = stringToLabel.find(str);
        if (it == stringToLabel.end()) {
            exit(999);
        } else
            return (*it).second;
    }
    int getTopParaCnt() {
        return paraCnt.top();
    }

    void setStack(int size) {
        paraCnt.push(size);
    }

    void funInitStack(const string& name, int size) {
        addTemporaryTable(name, size << 2);
    }

    void setPushCnt(int x) {
        pushCnt += x;
    }

    int getPushCnt() {
        pushCnt += 1;
        return pushCnt;
    }

    void setLayer(int det) {
        if (det < 0) {
            sp += (paraCnt.top() << 2);
            paraCnt.pop();
        } else {
            paraCnt.push(0);
        }
        layer += det;
    }

    int checkTable(const string& name) {
        if (name[0] == '@') {
            for (int i = 0; i < (int)table.size(); ++ i)
                if (table[i].getName() == name)
                    return i;
                else if (table[i].getLayer() > 0)
                    return -1;
            return -1;
        } else {
            for (int i = (int)table.size() - 1; i >= 0; -- i) {
                if (table[i].getLayer() < layer)
                    break;
                if (table[i].getName() == name) {
                    return i;
                }
            }
            return -1;
        }
    }

    void addGlobalTable(const string& name, int size) {
        cout << "global : " << name << " " << size << endl;
        table.emplace_back(name, layer, data, 0);
        data += size;
    }

    int addTemporaryTable(const string& name, int size) {
        sp -= size;
        table.emplace_back(name, layer, sp, 1);
        return (int)table.size() - 1;
    }

    int getTabelItemAdd(int index) {
        return table[index].getAdd();
    }

    bool getTabelItemTem(int index) {
        return table[index].getTemporary();
    }

    void getRegFromMem(const string& reg, const string& name, const string& offset) {
        int index = checkTable(name);
        cout << "get arr: " << name << " " << offset << endl;
        if (index == -1)
            exit(111);
        if (getTabelItemTem(index)) {
            int off = getTabelItemAdd(index) - sp;
            if (offset[0] == '$') {
                mipsOutput -> push_back(new MipsLoad("lw", reg, to_string(off), offset));
            } else {
                off += (atoi(offset.c_str()) << 2);
                mipsOutput -> push_back(new MipsLoad("lw", reg, to_string(off), "$0"));
            }
        } else {
            int off = getTabelItemAdd(index);
            if (offset[0] == '$') {
                mipsOutput -> push_back(new MipsLoad("lw", reg, to_string(off), offset));
            } else {
                off += (atoi(offset.c_str()) << 2);
                mipsOutput -> push_back(new MipsLoad("lw", reg, to_string(off), "$0"));
            }
        }
    }

    void getRegFromMem(const string& reg, const string& name) {
        if (name == "%0") {
            mipsOutput->push_back(new MipsAdd("add", reg, "$0", "$0"));
            return ;
        }
        int index = checkTable(name);
        cout << "get var: " << name << endl;
        if (index == -1)
            exit(222);
        if (getTabelItemTem(index)) {
            int offset = getTabelItemAdd(index) - sp;
            mipsOutput->push_back(new MipsLoad("lw", reg, offset, "$sp"));
        } else {
            mipsOutput->push_back(new MipsLoad("lw", reg, getTabelItemAdd(index), "$0"));
        }
    }

    void setRegToMem(const string& reg, const string& name) {
        int index = checkTable(name);
        cout << "set var: " << name << endl;
        if (index == -1)
            exit(333);
        if (getTabelItemTem(index)) {
            int offset = getTabelItemAdd(index) - sp;
            mipsOutput->push_back(new MipsStore("sw", reg, offset, "$sp"));
        } else {
            mipsOutput->push_back(new MipsStore("sw", reg, getTabelItemAdd(index), "$0"));
        }
    }

    void setRegToMem(const string& reg, const string& name, const string& offset) {
        int index = checkTable(name);
        cout << "set arr: " << name << " " << offset << endl;
        if (index == -1)
            exit(444);
        if (getTabelItemTem(index)) {
            int off = getTabelItemAdd(index) - sp;
            if (offset[0] == '$') {
                mipsOutput -> push_back(new MipsStore("lw", reg, to_string(off), offset));
            } else {
                off += (atoi(offset.c_str()) << 2);
                mipsOutput -> push_back(new MipsStore("lw", reg, to_string(off), "$0"));
            }
        } else {
            int off = getTabelItemAdd(index);
            if (offset[0] == '$') {
                mipsOutput -> push_back(new MipsStore("lw", reg, to_string(off), offset));
            } else {
                off += (atoi(offset.c_str()) << 2);
                mipsOutput -> push_back(new MipsStore("lw", reg, to_string(off), "$0"));
            }
        }
    }

    void setTopName(const string& name) {
        addTemporaryTable(name, 4);
    }

    void setFunBeginSp(int det) {
        sp += (det << 2);
        paraCnt.push(det);
    }

    static void moveSp(int det) {
        if (det > 0)
            mipsOutput->push_back(new MipsAddI("addi", "$sp", "$sp", to_string(det)));
        else
            mipsOutput->push_back(new MipsAddI("subi", "$sp", "$sp", to_string(-det)));
    }

    void getRegFromAddress(const string& reg, const string& name, int offset) {
        int index = checkTable(name);
        if (index == -1)
            exit(999);
        if (getTabelItemTem(index)) {
            int imm = getTabelItemAdd(index) - sp + (offset << 2);
            mipsOutput->push_back(new MipsAddI("addi", reg, "$sp", to_string(imm)));
        } else {
            int imm = getTabelItemAdd(index) + (offset << 2);
            mipsOutput->push_back(new MipsAddI("addi", reg, "$0", to_string(imm)));
        }
    }
};

#endif //COMPILER_MIPS_TABLE_H
