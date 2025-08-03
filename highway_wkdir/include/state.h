#ifndef STATE_H_
#define STATE_H_
#include "suan_png.h"
#include "pxl.h"

#define MAX 65536
#define INF 0x3f3f3f3f

struct State
{
    // data structure
    int visited[MAX];
    int pathLength[MAX]; // 存路径长度
    int minPath[MAX];    // 存最短路路径
    int secondMinPath;   // 次短路
    int deletedEdge;     // 删去的边
    int row;
    int column;
};

// function
void init_State(struct State *s);
void delete_State(struct State *s);
void parse(struct State *s, struct PNG *p);
int solve1(struct State *s);
int solve2(struct State *s);

#endif
