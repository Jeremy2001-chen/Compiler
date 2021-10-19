//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef COMPILER_NODE_H
#define COMPILER_NODE_H

class Node{
public:
    virtual void check() = 0;
    virtual void traversal() = 0;
};

#endif //COMPILER_NODE_H
