//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_SINGLE_EXP_H
#define COMPILER_SINGLE_EXP_H

#include "node.h"
#include <cassert>

#define lch ch[0]
class SingleExp: public Node {
private:
    Node ch[1] = {NULL};
    string sign;
public:
    string getSign() const {
        return sign;
    }
    void setLch(Node Lch) {
        lch = Lch;
    }
};

class UnaryExp: public SingleExp {
public:
    UnaryExp(string _sign) {
        sign = _sign;
    }
    void check() override {
        assert(sign == "+" || sign == "-" || sign == "!");
        cout << "UnaryExp check correct!" << endl;
        this.lch.check();
    }
    void traversal() override {
        cout << sign << endl;
        this.lch.traversal();
    }
};

#endif //COMPILER_SINGLE_EXP_H
