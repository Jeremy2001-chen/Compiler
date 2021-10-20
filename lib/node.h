//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef COMPILER_NODE_H
#define COMPILER_NODE_H

class Node{
private:
    int type = 0; //-1: void, 0: int, 1: int[], 2: int[][]
    bool Const = false;
    int line = 0;
public:
    virtual void check() = 0;
    virtual void traversal() = 0;
    int getType() const {
        return type;
    }
};

#endif //COMPILER_NODE_H
