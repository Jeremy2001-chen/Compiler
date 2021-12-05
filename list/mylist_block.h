//
// Created by chenjiyuan3 on 2021/12/5.
//

#ifndef COMPILER_MYLIST_BLOCK_H
#define COMPILER_MYLIST_BLOCK_H

#include "../ir/ir_block.h"

class MyListBlock{
private:
    MyListBlock *next, *prev;
    IrBlock* irBlock;
public:
    MyListBlock(IrBlock* _ir) {
        next = prev = nullptr;
        irBlock = _ir;
    }
    MyListBlock* getPrev() const {
        return prev;
    }
    MyListBlock* getNext() const {
        return next;
    }
    void setNext(MyListBlock* _next) {
        next = _next;
    }
    void setPrev(MyListBlock* _prev) {
        prev = _prev;
    }
    IrBlock* getBlock() const {
        return irBlock;
    }
    void linkNext(MyListBlock* _next) {
        next = _next;
        if (_next != nullptr)
            _next->setPrev(this);
    }
    void linkPrev(MyListBlock* _prev) {
        prev = _prev;
        if (_prev != nullptr)
            _prev->setNext(this);
    }
    MyListBlock* removeToPrev() {
        if (prev)
            prev -> linkNext(next);
        return prev;
    }
};

#endif //COMPILER_MYLIST_BLOCK_H
