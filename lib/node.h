//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef COMPILER_NODE_H
#define COMPILER_NODE_H

#include "../type.h"

class Node{
protected:
    int type = 0; //-2: NULL, -1: void, 0: int, 1: int[], 2: int[][]
    ClassType classType = NodeType;
    bool Const = false;
    int line = 0;
public:
    virtual void check() = 0;
    virtual void traversal() = 0;
    int getType() const {
        return type;
    }
    bool getConstType() const {
        return Const;
    }
    bool empty() const {
        return type == -2;
    }
    ClassType getClassType() const {
        return classType;
    }
    int getLine() const {
        return line;
    }
    void setType(int _type) {
        type = _type;
    }
};

#endif //COMPILER_NODE_H
