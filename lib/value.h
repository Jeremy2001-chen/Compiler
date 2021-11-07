//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include <utility>
#include "../ir/ir.h"
#include "../ir/ir_code.h"
#include "../ir/ir_table.h"

#include "node.h"

extern IR IR_1;

class ConstValue: public Node{
protected:
    int value;
public:
    Node* optimize() override {
        return this;
    }

    int getValue() const {
        return value;
    }
};

class Number: public ConstValue {
public:
    explicit Number(int _value) {
        value = _value;
        Const = true;
        classType = NumberType;
    }

    void check() override {
        cout << "ConstValue check correct!" << endl;
    }

    void traversal() override {
        string var = irTableList_1.allocTem();
        IR_1.add(new IrNumberAssign(var, to_string(value)));
    }
};

Node* getOffset(Node* offsetTree) {
    Node* off = offsetTree->optimize();
    if (off->getClassType() != NumberType && off->getClassType() != VariableType) {
        cout << off->getClassType() << endl;
        cout << "error when the offset not a const exp!" << endl;
        exit(-3);
    }
    return off;
}

class Variable: public ConstValue {
private:
    string name;
    int offset;
    Node* offsetTree;
    Node* valueTree = nullptr;
    bool isArray;
public:
    Variable(string _name, Node* _offsetTree, int _type, bool _const) {
        name = std::move(_name);
        //offset = _offset;
        if (_offsetTree == nullptr)
            offsetTree = new Number(1);
        else
            offsetTree = _offsetTree->optimize();
        type = _type;
        value = 0;
        Const = _const;
        classType = VariableType;
    }
    Variable(string _name, Node* _offsetTree, int _type, Node* _valueTree, bool _const, bool _isArray) {
        name = std::move(_name);
        if (_offsetTree == nullptr) {
            offsetTree = new Number(0);
            offset = 0;
        }
        else {
            offsetTree = _offsetTree->optimize();
            if (offsetTree -> getClassType() == NumberType) {
                offset = ((Number*)offsetTree) -> getValue();
            }
        }
        type = _type;

        if (_valueTree == nullptr) {
            value = 0;
            valueTree = nullptr;
        } else {
            value = ((ConstValue*)_valueTree) -> getValue();
            valueTree = nullptr;
        }
        Const = _const;
        classType = VariableType;
        isArray = _isArray;
    }
    void check() override {
        cout << "Variable check correct!" << endl;
    }
    void traversal() override {
        string irName = irTableList_1.getIrName(name);
        if (!isArray) {
            string var = irTableList_1.allocTem();
            IR_1.add(new IrBinaryOp(var, irName, "+", "%0"));
        } else {
            offsetTree -> traversal();
            string last = irTableList_1.getTopTemIrName(), var = irTableList_1.allocTem();
            IR_1.add(new IrArrayGet(var, irName, last));
        }
    }

    string getName() {
        return name;
    }

    Node* getOffsetTree() {
        return offsetTree;
    }

    bool getIsArray() {
        return isArray;
    }
};

class VariableDecl: public Node {
private:
    string name;
    int offset;
    Node* offsetTree;
    vector<Node*>* value;
    int size;
public:
    Node* optimize() override {
        return this;
    }
    VariableDecl(string _name, vector<Node*>* set, Node* _offsetTree, vector<Node*>* _value, int _type, bool _const, int _line) {
        name = std::move(_name);
        if (_offsetTree == nullptr) {
            offsetTree = new Number(0);
            offset = 0;
        } else {
            offsetTree = getOffset(_offsetTree);
            offset = ((Number*)offsetTree)->getValue();
        }
        value = _value;
        if (value == nullptr) {
            value = new vector<Node*>();
        }
        if (set == nullptr)
            size = -1;
        else {
            size = 1;
            for (auto s: (*set)) {
                size *= ((ConstValue*)s) -> getValue();
            }
        }
        /*
        for (int i = 0; i < value->size(); ++ i) {
            if ((*value)[i]->getConstType())
                cout << name << " " << i << " " << ((ConstValue*)(*value)[i])->getValue() << endl;
        }*/
        Const = _const;
        line = _line;
        type = _type;
        classType = VariableDeclType;
    }
    void check() override {
        cout << "Variable Declaration check correct!" << endl;
    }
    void traversal() override {
        if (size > 0) {
            string irName = irTableList_1.addVar(name);
            if (type == 0) {
                if ((*value).empty())
                    IR_1.add(new IrVarDefineWithOutAssign(Const, irName));
                else {
                    (*value)[0] -> traversal();
                    string tem = irTableList_1.getTopTemIrName();
                    IR_1.add(new IrVarDefineWithAssign(Const, irName, tem));
                }
            } else {
                if (!value->empty()) {
                    auto* va = new vector<int>();
                    for (auto i: *value) {
                        if (i->getClassType() == NumberType) {
                            va->push_back(((Number*)i) -> getValue());
                        } else
                            va->push_back(0);
                    }
                    IR_1.add(new IrArrayDefineWithAssign(Const, irName, va));
                    for (int i = 0; i < (*value).size(); ++ i) {
                        if ((*value)[i]->getClassType() != NumberType) {
                            (*value)[i] -> traversal();
                            IR_1.add(new IrArrayAssign(irName, to_string(i), irTableList_1.getTopTemIrName()));
                        }
                    }
                } else
                    IR_1.add(new IrArrayDefineWithOutAssign(Const, irName, to_string(size)));
            }
        }

        /*cout << name << "[0]~" << name << "[" << offset - 1 << "] has declare!" << endl;
        for (auto &node: value) {
            node.traversal();
        }*/
    }
    string getName() const {
        return name;
    }
    Node* getOffsetTree() {
        return offsetTree;
    }
    Node* getPosValue(int pos) {
        if (pos < (int)value->size())
            return (*value)[pos];
        return new Number(0); //ub
    }
};

class ReadValue: public Node{
public:
    Node* optimize() override {
        return this;
    }
    ReadValue() {
        classType = ReadValueType;
    }
    void check() override {
        //cout << "please read a integer!" << endl;
    }
    void traversal() override {

    }
};

class DeclStmt: public Node {
private:
    vector<VariableDecl*>* decl;
public:
    Node* optimize() override {
        return this;
    }
    DeclStmt() {
        decl = new vector<VariableDecl*>();
        classType = DeclStmtType;
    }
    void addDecl(VariableDecl* varDecl) {
        decl->push_back(varDecl);
    }
    void check() override {
        //cout << "please read a integer!" << endl;
    }
    void traversal() override {
        for (auto dec: (*decl)) {
            dec->traversal();
        }
    }
    vector<VariableDecl*>* getDecl() {
        return decl;
    }
};
#endif //COMPILER_VALUE_H
