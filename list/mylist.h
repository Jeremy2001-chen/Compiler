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
        if (_next != nullptr)
            _next->setPrev(this);
    }
    void linkPrev(MyList* _prev) {
        prev = _prev;
        if (_prev != nullptr)
            _prev->setNext(this);
    }
    MyList* removeToPrev() {
        if (prev)
            prev -> linkNext(next);
        return prev;
    }
};

#endif //COMPILER_LIST_H
