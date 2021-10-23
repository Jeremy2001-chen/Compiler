//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include <utility>

#include "node.h"

class Number: public Node {
private:
    int value;
public:
    Number(int _value) {
        value = _value;
        Const = true;
        classType = NumberType;
    }
    void check() override {
        cout << "ConstValue check correct!" << endl;
    }
    void traversal() override {
        //cout << value << endl;
    }
};

class Variable: public Node {
private:
    string name;
    int offset;
    Node* offsetTree;
    int value;
    Node* valueTree;
public:
    Variable(string _name, Node* _offsetTree, int _type, bool _const) {
        name = std::move(_name);
        //offset = _offset;
        offsetTree = _offsetTree;
        type = _type;
        value = 0;
        Const = _const;
        classType = VariableType;
    }
    Variable(string _name, Node* _offsetTree, int _type, Node* _valueTree, bool _const) {
        name = std::move(_name);
        offsetTree = _offsetTree;
        //offset = _offset;
        type = _type;
        valueTree = _valueTree;
        //value = 0;
        Const = _const;
        classType = VariableType;
    }
    void check() override {
        cout << "Variable check correct!" << endl;
    }
    void traversal() override {
        //cout << name << "[" << offset << "] = " << value << endl;
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
    VariableDecl(string _name, Node* _offsetTree, vector<Node*>* _value, int _type, bool _const, int _line) {
        name = std::move(_name);
        offsetTree = _offsetTree;
        offset = 0; //reserve
        value = _value;
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
};

class ReadValue: public Node{
public:
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
