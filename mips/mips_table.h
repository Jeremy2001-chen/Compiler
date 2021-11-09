//
// Created by chenjiyuan3 on 2021/11/9.
//

#ifndef COMPILER_MIPS_TABLE_H
#define COMPILER_MIPS_TABLE_H

class MipsTableItem {
private:
    string name;
    int layer;
    unsigned int add;

public:
    MipsTableItem(string _name, int _layer, unsigned int _add) {
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
    unsigned int getAdd() const {
        return add;
    }
};

class MipsTable {
private:
    int layer = 0;
    vector<MipsTableItem> table;

public:
    MipsTable() = default;

    void setLayer(int det) {
        if (det < 0) {
            for (int i = (int)table.size() - 1; i >= 0; -- i) {
                if (table[i].getLayer() == layer)
                    table.pop_back();
            }
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

    void addTable(const string& name, unsigned int _add) {
        table.emplace_back(name, layer, _add);
    }

    unsigned int getTabelItemAdd(int index) {
        return table[index].getAdd();
    }
};

#endif //COMPILER_MIPS_TABLE_H
