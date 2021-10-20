//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_BLOCK_H
#define COMPILER_BLOCK_H

#include <vector>
class Block: public Node{
private:
    vector<Node> blockItem;
public:
    Block() = default;
    void addBlockItem(Node node) {
        blockItem.push_back(node);
    }
    void check() override {
        assert(blockItem);
        for (auto blockIt: blockItem) {
            blockIt.check();
        }
        cout << "Block check correct!" << endl;
    }
    void traversal() override {
        for (auto blockIt: blockItem) {
            blockIt.traversal();
        }
    }
};
#endif //COMPILER_BLOCK_H
