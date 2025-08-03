#include "state.h"
#include <string.h>

typedef struct Point
{
    int weight; // 点的权值
} PointWeight;  // 州的点集合

PointWeight Point[MAX];
int path[MAX];

typedef struct EdgeNode
{
    int vertex; // 终点坐标
    int next;
} Edge;

Edge edge[MAX];  // 边数组
int head[MAX];   // 头结点
int edgeNum = 0; // 边的数量
int nodeNum = 0;

void init_State(struct State *s)
{
    for (int i = 0; i <= MAX; i++)
    {
        s->pathLength[i] = INF;
        s->visited[i] = 0;
        s->minPath[i] = 0;
        s->secondMinPath = INF;
    }
    s->row = 0, s->column = 0; // 初始化
    return;
}

void delete_State(struct State *s)
{
    // TODO
}

int max(int x, int y)
{
    return x > y ? x : y;
}

void insertEdge(int u, int v)
{
    edge[++edgeNum].vertex = v;
    edge[edgeNum].next = head[u];
    head[u] = edgeNum;
}

void buildEdge(struct State *s, int weight, int column)
{
    int maxLine = s->column;
    Point[++nodeNum].weight = weight;
    if (s->row % 2 == 0)
    {
        insertEdge(nodeNum, nodeNum - maxLine);
        insertEdge(nodeNum - maxLine, nodeNum);
        insertEdge(nodeNum, nodeNum - maxLine + 1);
        insertEdge(nodeNum - maxLine + 1, nodeNum);
    }
    if (s->row % 2 == 1)
    {
        if (s->row > 1)
        {
            if (column < maxLine)
            {
                insertEdge(nodeNum, nodeNum - (maxLine - 1));
                insertEdge(nodeNum - (maxLine - 1), nodeNum);
            }
            if (column > 1)
            {
                insertEdge(nodeNum, nodeNum - (maxLine - 1) - 1);
                insertEdge(nodeNum - (maxLine - 1) - 1, nodeNum);
            }
        }
    }
    if (column > 1)
    {
        insertEdge(nodeNum, nodeNum - 1);
        insertEdge(nodeNum - 1, nodeNum);
    }
}

int calculateWeight(struct PNG *p, int w, int h)
{
    int r, g, b;
    r = get_PXL(p, w, h)->red;
    g = get_PXL(p, w, h)->green;
    b = get_PXL(p, w, h)->blue;
    return 255 * 255 * 3 - r * r - g * g - b * b;
}

void parse(struct State *s, struct PNG *p)
{
    int height = get_height(p);
    int width = get_width(p);
    int weight;
    s->row = 1;
    for (int h = 6; h < height; h += 8)
    {
        int line = 1;
        for (int w = 6; w < width; w += 8)
        {
            weight = calculateWeight(p, w, h);
            if (weight == 0)
                continue;
            buildEdge(s, weight, line);
            line++;
        }
        if (s->row == 1)
        {
            s->column = line - 1;
        }
        line = 1;
        (s->row)++;
    }
    return;
}

int solve1(struct State *s)
{
    // TODO
    s->visited[1] = 1;
    s->pathLength[1] = 0;
    s->minPath[1] = -1;
    int k = 1;            // 加入路径的点的数量
    int currentPoint = 1; // 当前所选择的点
    while (k < nodeNum)
    {
        for (int i = head[currentPoint]; i != 0; i = edge[i].next)
        {
            if (!s->visited[edge[i].vertex] && s->pathLength[edge[i].vertex] > s->pathLength[currentPoint] + Point[edge[i].vertex].weight)
            {
                s->pathLength[edge[i].vertex] = s->pathLength[currentPoint] + Point[edge[i].vertex].weight;
                s->minPath[edge[i].vertex] = currentPoint;
            }
        }
        int minDistance = INF;
        for (int i = 1; i <= nodeNum; i++)
        {
            if (!s->visited[i] && minDistance > s->pathLength[i])
            {
                currentPoint = i;
                minDistance = s->pathLength[i];
            }
        }
        s->visited[currentPoint] = 1;
        s->pathLength[currentPoint] = minDistance;
        k++;
    }
    return s->pathLength[nodeNum];
}

int solve2(struct State *s)
{
    // TODO
    int u = 0; //
    int minLength = s->pathLength[nodeNum];
    for (int i = nodeNum; i != -1; i = s->minPath[i])
    {
        u = s->minPath[i];
        int tempEdge = 0;
        for (int p = head[u]; p; p = edge[p].next)
        {
            if (edge[p].vertex == i)
            {
                tempEdge = p;
                break;
            }
        }
        s->deletedEdge = tempEdge; // 删除边
        // dijkstra
        memset(s->visited, 0, sizeof(s->visited));
        for (int i = 0; i <= MAX; i++)
        {
            s->pathLength[i] = INF;
        }
        s->visited[1] = 1;
        s->pathLength[1] = 0;
        int k = 1;            // 加入路径的点的数量
        int currentPoint = 1; // 当前所选择的点
        while (k < nodeNum)
        {
            for (int i = head[currentPoint]; i != 0; i = edge[i].next)
            {
                if (i != s->deletedEdge && !s->visited[edge[i].vertex] && s->pathLength[edge[i].vertex] > s->pathLength[currentPoint] + Point[edge[i].vertex].weight)
                {
                    s->pathLength[edge[i].vertex] = s->pathLength[currentPoint] + Point[edge[i].vertex].weight;
                }
            }
            int minDistance = INF;
            for (int i = 1; i <= nodeNum; i++)
            {
                if (!s->visited[i] && minDistance > s->pathLength[i])
                {
                    currentPoint = i;
                    minDistance = s->pathLength[i];
                }
            }
            if (minDistance == INF)
                break;
            s->visited[currentPoint] = 1;
            s->pathLength[currentPoint] = minDistance;
            k++;
        }
        if (s->pathLength[nodeNum] > minLength && s->secondMinPath > s->pathLength[nodeNum])
        {
            s->secondMinPath = s->pathLength[nodeNum];
        }
    }
    return s->secondMinPath;
}
