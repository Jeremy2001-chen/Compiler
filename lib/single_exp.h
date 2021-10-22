//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_SINGLE_EXP_H
#define COMPILER_SINGLE_EXP_H

#include "node.h"
#include <cassert>
#include <utility>

#define lch ch[0]
class SingleExp: public Node {
protected:
    Node* ch[1]{};
    string sign;
public:
    SingleExp() {
        classType = SingleExpType;
        lch = nullptr;
    }
    string getSign() const {
        return sign;
    }
    void setLch(Node* Lch) {
        lch = Lch;
    }
    void check() override {
        cout << "SingleExp check correct!" << endl;

    }
    void traversal() override {
    }
};

class UnaryExp: public SingleExp {
public:
    UnaryExp(string _sign) {
        sign = std::move(_sign);
        classType = UnaryExpType;
    }
    void check() override {
        assert(sign == "+" || sign == "-" || sign == "!");
        cout << "UnaryExp check correct!" << endl;
        //lch->check();
    }
    void traversal() override {
        /*cout << sign << endl;
        lch->traversal();*/
    }
};

#endif //COMPILER_SINGLE_EXP_H
