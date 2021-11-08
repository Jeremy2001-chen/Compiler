//
// Created by chenjiyuan3 on 2021/10/21.
//

#ifndef COMPILER_TABLE_H
#define COMPILER_TABLE_H

#include <utility>
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
        name = std::move(_name);
        kind = std::move(_kind);
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
    int getLayer() const {
        return layer;
    }
    Node* getAstNode() {
        return astNode;
    }
};

class SymbolTable {
private:
    vector<Table*> list;
    int tableSize = 0, maxSize = 0;
    int layer = 0;
public:
    void insertFunTable(const string& _name, Node* _node, int line) {
        if (!checkDecl(_name, "fun")) {
            output.addError(new NameRedefineError(line, _name));
        } else {
            cout << "add to Table: " << _name << " " << "fun" << endl;
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
    void insertVarTable(const string& _name, Node* _node, bool isConst, int line) {
        if (!checkDecl(_name, "var")) {
            output.addError(new NameRedefineError(line, _name));
        } else {
            cout << "add to Table: " << _name << " " << "var" << endl;
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
    int findTableDecl(const string& name, const string& kind) {
        int index = -1;
        for (int i = tableSize - 1; i >= 0; -- i) {
            if (list[i]->getLayer() != layer)
                break;
            else if (list[i]->getName() == name && list[i]->getKind() == kind) {
                index = i;
                break;
            }
        }
        return index;
    }
    int findTableUse(const string& name, const string& kind) {
        int index = -1;
        for (int i = tableSize - 1; i >= 0; -- i) {
            if (list[i]->getName() == name && list[i]->getKind() == kind) {
                index = i;
                break;
            }
        }
        return index;
    }
    bool checkDecl(const string& _name, const string& _kind) {
        return findTableDecl(_name, _kind) == -1;
    }
    bool checkUse(const string& _name, const string& _kind) {
        return findTableUse(_name, _kind) != -1;
    }
    Table* getUse(const string& _name, const string& _kind) {
        return getIndex(findTableUse(_name, _kind));
    }
    void pop() {
        tableSize -= 1;
    }
    void addLayer() {
        layer += 1;
    }
    void popLayer() {
        int newSize = tableSize;
        for(int i = tableSize - 1; i >= 0; -- i)
            if (list[i]->getLayer() == layer) {
                cout << "remove: " << list[i]->getName() << " " << list[i]->getKind() << endl;
                newSize -= 1;
            }
        tableSize = newSize;
        layer -= 1;
    }
    int getLayer() const {
        return layer;
    }
    Table* getIndex(int index) {
        if (index < 0 || index > tableSize) {
            return nullptr;
        }
        return list[index];
    }
    Table* getTableFromName(const string& _name, int line) {
        for (int i = tableSize - 1 ; i >= 0; -- i) {
            if (list[i]->getName() == _name)
                return list[i];
        }
        output.addError(new UndefineNameError(line, _name));
        return nullptr;
    }
    Table* getTopFun() {
        for (int i = tableSize - 1; i >= 0; -- i) {
            if (list[i]->getKind() == "fun") {
                return list[i];
            }
        }
        cout << "error when get top fun" << endl;
        exit(1);
    }
};
#endif //COMPILER_TABLE_H
