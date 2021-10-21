//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef COMPILER_NODE_H
#define COMPILER_NODE_H

class Node{
protected:
    int type = 0; //-2: NULL, -1: void, 0: int, 1: int[], 2: int[][]
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
};

#endif //COMPILER_NODE_H
