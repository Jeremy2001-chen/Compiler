//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_VALUE_H
#define COMPILER_VALUE_H

#include "node.h"

class Number : public Node {
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
        cout << value << endl;
    }
};

class Variable : public Node {
private:
    string name;
    int offset;
    int value;
    Variable(string _name, int _offset, int _type, bool _const) {
        name = _name;
        offset = _offset;
        type = _type;
        value = 0;
        Const = _const;
    }
    Variable(string _name, int _offset, int _type, int _value, bool _const) {
        name = _name;
        offset = _offset;
        type = _type;
        value = _value;
        Const = _const;
    }
    void check() override {
        cout << "Variable check correct!" << endl;
    }
    void traversal() override {
        cout << name << "[" << offset << "] = " << value << endl;
    }
};

class
#endif //COMPILER_VALUE_H
