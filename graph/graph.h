//
// Created by chenjiyuan3 on 2021/12/2.
//

#ifndef COMPILER_GRAPH_H
#define COMPILER_GRAPH_H

#include <set>
#include <queue>

#define dom(x) (*(dominate[x]))

class Graph {
private:
    int point;
    vector<int> first;
    vector<vector<int>* > edges;

    vector<vector<int>* > dominate; //DOM(Y): X dominate Y means that every path from entry to Y all go through X
    vector<set<int>* > df; //DF(Y) = X dominate the prev of Y but not dominate Y

    vector<bool> reach;
    vector<bool> flag;

public:
    explicit Graph(int N) {
        point = N;
        first.resize(N);
        for (int i = 0; i < N; ++ i) {
            auto* edge = new vector<int>();
            edges.push_back(edge);
        }
        reach.resize(N);
        flag.resize(N);
        for (int i = 0; i < N; ++ i) {
            auto* dom = new vector<int>(N);
            dominate.push_back(dom);
            auto* dfp = new set<int>();
            df.push_back(dfp);
        }
    }

    void link(int s, int t) {
        cout << "link : " << s << " " << t << endl;
        edges[s] -> push_back(t);
    }

    void bfs(int st) {
        queue<int> Q;
        if (st > 0) Q.push(0);
        for(int i = 1; i < point; ++ i)
            reach[i] = false;
        reach[0] = reach[st] = true;
        while (!Q.empty()) {
            int now = Q.front();Q.pop();
            for (auto nx: *edges[now]) {
                if (!reach[nx]) {
                    reach[nx] = true;
                    Q.push(nx);
                }
            }
        }
        reach[st] = false;
        for (int i = 0; i < point; ++ i)
            dom(st)[i] = 1 - reach[i];
    }

    void getDF(int st) {
        for (int i = 0; i < point; ++ i)
            if (dom(st)[i]) {
                for (auto nx: *edges[i]) {
                    if (!dom(st)[nx])
                        (*df[st]).insert(nx);
                }
            }
    }

    void debug() {
        cout << "-----------------------dom---------------------" << endl;
        for (int i = 0; i < point; ++ i) {
            cout << "dom : " << i << endl;
            for (int j = 0; j < point; ++ j)
                if (dom(i)[j]) {
                    cout << j << " ";
                }
            cout << endl;
        }


        cout << "-----------------------df---------------------" << endl;
        for (int i = 0; i < point; ++ i) {
            cout << "df : " << i << endl;
            for (auto c: (*df[i])) {
                cout << c << " ";
            }
            cout << endl;
        }
    }

    void ssaInit() {
        for (int i = 0; i < point; ++ i)
            bfs(i);
        for (int i = 0; i < point; ++ i)
            getDF(i);
        debug();
    }
};
#endif //COMPILER_GRAPH_H