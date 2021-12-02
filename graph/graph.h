//
// Created by chenjiyuan3 on 2021/12/2.
//

#ifndef COMPILER_GRAPH_H
#define COMPILER_GRAPH_H

class Graph {
private:
    int point;
    vector<int> first;
    vector<vector<int>* > edges;
public:
    Graph(int N) {
        point = N;
        first.resize(N);
        for (int i = 0; i < N; ++ i) {
            auto* edge = new vector<int>();
            edges.push_back(edge);
        }
    }

    void link(int s, int t) {
        cout << "link : " << s << " " << t << endl;
        edges[s] -> push_back(t);
    }
};
#endif //COMPILER_GRAPH_H
