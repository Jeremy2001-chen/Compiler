////
//// Created by chenjiyuan3 on 2021/12/7.
////
//
//#include <bits/stdc++.h>
//
//using namespace std;
//
//#include "ir_new.h"
//
//void removeAddZero(IrNew* irNew) {
//    auto* funList = irNew->getIrFun();
//    for (auto c: *funList) {
//        auto* block = c -> getFirstBlock();
//        while (block != nullptr) {
//            IrBlock* block1 = block -> getBlock();
//            MyList* start = block1 -> getStartCode();
//            while (start != nullptr) {
//                IrCode* code = start -> getCode();
//                if (code -> getCodeType() == IrBinaryOpType) {
//                    IrBinaryOp* binary = (IrBinaryOp*)code;
//                    string sign = binary -> getSign();
//                    if (binary -> getSource(1) == "%0" && (sign == "+" || sign == "-")) {
//                        IrUnaryOp* unaryOp = new IrUnaryOp(binary -> getTarget(), "+", binary -> getSource(0));
//                        MyList* myList = new MyList(unaryOp);
//                        block1 -> replace(start, myList);
//                        start = myList;
//                    }
//                }
//                start = start -> getNext();
//            }
//        }
//    }
//}
