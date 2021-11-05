//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include <utility>

#include "node.h"

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
        cout << value << endl;
    }
};

Node* getOffset(Node* offsetTree) {
    Node* off = offsetTree->optimize();
    if (off->getClassType() != NumberType && off->getClassType() != VariableType) {
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
    Variable(string _name, Node* _offsetTree, int _type, Node* _valueTree, bool _const) {
        name = std::move(_name);
        if (_offsetTree == nullptr) {
            offsetTree = new Number(1);
            offset = 1;
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
    }
    void check() override {
        cout << "Variable check correct!" << endl;
    }
    void traversal() override {
        cout << name << "[" << offset << "] = " << value << endl;
    }
    string getName() {
        return name;
    }
};

class VariableDecl: public Node {
private:
    string name;
    int offset;
    Node* offsetTree;
    vector<Node*>* value;
public:
    Node* optimize() override {
        return this;
    }
    VariableDecl(string _name, Node* _offsetTree, vector<Node*>* _value, int _type, bool _const, int _line) {
        name = std::move(_name);
        if (_offsetTree == nullptr) {
            offsetTree = new Number(1);
            offset = 1;
        } else {
            offsetTree = getOffset(_offsetTree);
            offset = ((Number*)offsetTree)->getValue();
        }
        value = _value;
        if (value == nullptr) {
            value = new vector<Node*>();
        }
        for (int i = 0; i < value->size(); ++ i) {
            if ((*value)[i]->getConstType())
                cout << name << " " << i << " " << ((ConstValue*)(*value)[i])->getValue() << endl;
        }
        Const = _const;
        line = _line;
        type = _type;
        classType = VariableDeclType;
    }
    void check() override {
        cout << "Variable Declaration check correct!" << endl;
    }
    void traversal() override {
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

    }
    vector<VariableDecl*>* getDecl() {
        return decl;
    }
};
#endif //COMPILER_VALUE_H
