//
// Created by chenjiyuan3 on 2021/12/1.
//

#ifndef COMPILER_LIST_H
#define COMPILER_LIST_H

class MyList{
private:
    MyList *next, *prev;
    IrCode* irCode;
public:
    MyList(IrCode* _ir) {
        next = prev = nullptr;
        irCode = _ir;
    }
    MyList* getPrev() const {
        return prev;
    }
    MyList* getNext() const {
        return next;
    }
    void setNext(MyList* _next) {
        next = _next;
    }
    void setPrev(MyList* _prev) {
        prev = _prev;
    }
    IrCode* getCode() const {
        return irCode;
    }
    void linkNext(MyList* _next) {
        next = _next;
        _next->setPrev(this);
    }
    void linkPrev(MyList* _prev) {
        prev = _prev;
        _prev->setNext(this);
    }
};

#endif //COMPILER_LIST_H
