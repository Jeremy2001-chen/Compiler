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
    int value;
    Variable(string _name, int _offset, int _type, bool _const) {
        name = std::move(_name);
        offset = _offset;
        type = _type;
        value = 0;
        Const = _const;
    }
    Variable(string _name, int _offset, int _type, int _value, bool _const) {
        name = std::move(_name);
        offset = _offset;
        type = _type;
        value = _value;
        Const = _const;
    }
    void check() override {
        cout << "Variable check correct!" << endl;
    }
    void traversal() override {
        //cout << name << "[" << offset << "] = " << value << endl;
    }
};

class VariableDecl: public Node {
private:
    string name;
    int offset;
    vector<Node*> value;
public:
    VariableDecl(string _name, int _offset, vector<Node*> _value, bool _const) {
        name = std::move(_name);
        offset = _offset;
        value = std::move(_value);
        Const = _const;
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
};

class ReadValue: public Node{
public:
    ReadValue() = default;
    void check() override {
        //cout << "please read a integer!" << endl;
    }
    void traversal() override {

    }
};
#endif //COMPILER_VALUE_H
