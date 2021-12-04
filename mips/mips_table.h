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
    bool temporary, isAddress;

public:
    MipsTableItem(string _name, int _layer, int _add, bool _tem, bool _isAdd) {
        name = std::move(_name);
        layer = _layer;
        add = _add;
        temporary = _tem;
        isAddress = _isAdd;
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
    bool getIsAddress() const {
        return isAddress;
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

    int funInitStack(const string& name, int size, bool _isAdd) {
        for (int i = (int)table.size() - 1; i >= 0; -- i) {
            //cout << i << " " << table[i].getLayer() << endl;
            if (table[i].getLayer() < layer)
                break;
            if (table[i].getName() == name) {
                return 0;
            }
        }
        //cout << "check : " << name << " " << size << endl;
        addTemporaryTable(name, size << 2, _isAdd);
        return size;
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
            sp += paraCnt.top();
            paraCnt.pop();
            for (int i = (int)table.size() - 1; i >= 0; -- i)
                if (table[i].getLayer() == layer)
                    table.pop_back();
                else
                    break;
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
        //cout << "global : " << name << " " << data << endl;
        table.emplace_back(name, layer, data, 0, false);
        data += size;
    }

    int addTemporaryTable(const string& name, int size, bool _isAdd) {
        sp -= size;
        table.emplace_back(name, layer, sp, 1, _isAdd);
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
        //cout << "get arr: " << name << " " << offset << endl;
        if (index == -1)
            exit(111);
        if (table[index].getTemporary()) {
            int off = getTabelItemAdd(index) - sp;
            if (table[index].getIsAddress()) {
                if (offset[0] == '$') {
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$sp", to_string(off)));
                    mipsOutput -> push_back(new MipsLoad("lw", "$t9", "0", "$t9"));
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$t9", offset));
                    mipsOutput -> push_back(new MipsLoad("lw", reg, "0", "$t9"));
                } else {
                    int of = (atoi(offset.c_str()) << 2);
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$sp", to_string(off)));
                    mipsOutput -> push_back(new MipsLoad("lw", "$t9", "0", "$t9"));
                    mipsOutput -> push_back(new MipsLoad("lw", reg, to_string(of), "$t9"));
                }
            } else {
                if (offset[0] == '$') {
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$sp", offset));
                    mipsOutput -> push_back(new MipsLoad("lw", reg, off, "$t9"));
                } else {
                    off += (atoi(offset.c_str()) << 2);
                    mipsOutput -> push_back(new MipsLoad("lw", reg, off, "$sp"));
                }
            }
        } else {
            int off = getTabelItemAdd(index);
            if (offset[0] == '$') {
                mipsOutput -> push_back(new MipsAdd("add", "$t9", "$0", offset));
                mipsOutput -> push_back(new MipsLoad("lw", reg, off, "$t9"));
            } else {
                off += (atoi(offset.c_str()) << 2);
                mipsOutput -> push_back(new MipsLoad("lw", reg, off, "$0"));
            }
        }
    }

    void getRegFromMem(const string& reg, const string& name) {
        if (name == "%0") {
            mipsOutput->push_back(new MipsAdd("add", reg, "$0", "$0"));
            return ;
        }
        int index = checkTable(name);
        //cout << "get var: " << name << endl;
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
        //cout << "set var: " << name << endl;
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
        //cout << "set arr: " << name << " " << offset << endl;
        if (index == -1)
            exit(444);
        if (table[index].getTemporary()) {
            int off = getTabelItemAdd(index) - sp;
            if (table[index].getIsAddress()) {
                if (offset[0] == '$') {
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$sp", to_string(off)));
                    mipsOutput -> push_back(new MipsLoad("lw", "$t9", "0", "$t9"));
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$t9", offset));
                    mipsOutput -> push_back(new MipsStore("sw", reg, "0", "$t9"));
                } else {
                    int of = (atoi(offset.c_str()) << 2);
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$sp", to_string(off)));
                    mipsOutput -> push_back(new MipsLoad("lw", "$t9", "0", "$t9"));
                    mipsOutput -> push_back(new MipsStore("sw", reg, to_string(of), "$t9"));
                }
            } else {
                if (offset[0] == '$') {
                    mipsOutput -> push_back(new MipsAdd("add", "$t9", "$sp", offset));
                    mipsOutput -> push_back(new MipsStore("sw", reg, off, "$t9"));
                } else {
                    off += (atoi(offset.c_str()) << 2);
                    mipsOutput -> push_back(new MipsStore("sw", reg, off, "$sp"));
                }
            }
        } else {
            int off = getTabelItemAdd(index);
            if (offset[0] == '$') {
                mipsOutput -> push_back(new MipsAdd("add", "$t9", "$0", offset));
                mipsOutput -> push_back(new MipsStore("sw", reg, off, "$t9"));
            } else {
                off += (atoi(offset.c_str()) << 2);
                mipsOutput -> push_back(new MipsStore("sw", reg, off, "$0"));
            }
        }
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

    void getRegFromAddress(const string& reg, const string& name, const string& offset) {
        int index = checkTable(name);
        if (index == -1)
            exit(999);
        if (table[index].getTemporary()) {
            if (table[index].getIsAddress()) {
                if (offset[0] >= '0' && offset[0] <= '9') {
                    int imm = getTabelItemAdd(index) - sp, off = (atoi(offset.c_str()) << 2);
                    mipsOutput->push_back(new MipsAddI("addi", "$t9", "$sp", to_string(imm)));
                    mipsOutput->push_back(new MipsLoad("lw", "$t9", "0", "$t9"));
                    mipsOutput->push_back(new MipsAddI("addi", reg, "$t9", to_string(off)));
                } else {
                    int imm = getTabelItemAdd(index) - sp;
                    mipsOutput->push_back(new MipsAddI("addi", "$t9", "$sp", to_string(imm)));
                    mipsOutput->push_back(new MipsLoad("lw", "$t9", "0", "$t9"));
                    mipsOutput->push_back(new MipsAddI("add", reg, "$t9", offset));
                }
            }
            else {
                if (offset[0] >= '0' && offset[0] <= '9') {
                    int imm = getTabelItemAdd(index) - sp + (atoi(offset.c_str()) << 2);
                    mipsOutput->push_back(new MipsAddI("addi", reg, "$sp", to_string(imm)));
                } else {
                    int imm = getTabelItemAdd(index) - sp;
                    mipsOutput->push_back(new MipsAdd("add", "$t9", "$sp", offset));
                    mipsOutput->push_back(new MipsAddI("addi", reg, "$t9", to_string(imm)));
                }
            }
        } else {
            if (offset[0] >= '0' && offset[0] <= '9') {
                int imm = getTabelItemAdd(index) + (atoi(offset.c_str()) << 2);
                mipsOutput->push_back(new MipsAddI("addi", reg, "$0", to_string(imm)));
            } else {
                int imm = getTabelItemAdd(index);
                mipsOutput->push_back(new MipsAddI("add", reg, offset, to_string(imm)));
            }
        }
    }
};

#endif //COMPILER_MIPS_TABLE_H
