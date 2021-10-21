//
// Created by chenjiyuan3 on 2021/10/21.
//

#ifndef COMPILER_TABLE_H
#define COMPILER_TABLE_H

#include <vector>
#include "./lib/function.h"
#include "output.h"
#include "error.h"
using namespace std;

extern Output output;
class Table{
private:
    string name, kind;
    int type, line, layer;
    bool constFlag;
    Node* astNode;
public:
    Table(string _name, string _kind, int _type, bool _constFlag, Node* _node, int _layer, int _line) {
        name = _name;
        kind = _kind;
        type = _type;
        line = _line;
        astNode = _node;
        constFlag = _constFlag;
        layer = _layer;
    }
    string getName() {
        return name;
    }
    string getKind() {
        return kind;
    }
    int getType() {
        return type;
    }
    int getLine() {
        return line;
    }
    bool getConstFlag() {
        return constFlag;
    }
    int getLayer() {
        return layer;
    }
};

class SymbolTable {
private:
    vector<Table*> list;
    int tableSize = 0, maxSize = 0;
    int layer;
public:
    void insertFunTable(const string& _name, FunF* _node, int line) {
        if (!checkDecl(_name)) {
            output.addError(NameRedefineError(line, _name));
        } else {
            Table* tableItem = new Table(_name, "fun", _node->getType(), _node->getConstType(), _node, layer, line);
            if (tableSize == maxSize) {
                list.push_back(tableItem);
                tableSize += 1;
                maxSize += 1;
            } else {
                list[tableSize] = tableItem;
                tableSize += 1;
            }
        }
    }
    void insertVarTable(const string& _name, VariableDecl* _node, bool isConst, int line) {
        if (!checkDecl(_name)) {
            output.addError(NameRedefineError(line, _name));
        } else {
            Table* tableItem = new Table(_name, "var", _node->getType(), isConst, _node, layer, line);
            if (tableSize == maxSize) {
                list.push_back(tableItem);
                tableSize += 1;
                maxSize += 1;
            } else {
                list[tableSize] = tableItem;
                tableSize += 1;
            }
        }
    }
    int findTable(const string& name) {
        int index = -1;
        for (int i = tableSize; i >= 0; -- i) {
            if (list[i]->getLayer() != layer)
                break;
            else if (list[i]->getName() == name) {
                index = i;
                break;
            }
        }
        return index;
    }
    bool checkDecl(const string& _name) {
        return findTable(_name) == -1;
    }
    void pop() {
        tableSize -= 1;
    }
    void setLayer(int det) {
        layer += det;
    }
    int getLayer() const {
        return layer;
    }
    Table* getIndex(int index) {
        if (index < 0 || index > tableSize) {
            exit(1);
        }
        return list[index];
    }
    Table* getTableFromName(string _name, int line) {
        for (int i = tableSize; i >= 0; -- i) {
            if (list[i]->getName() == _name)
                return list[i];
        }
        output.addError(UndefineNameError(line, _name));
        return nullptr;
    }
};
#endif //COMPILER_TABLE_H
