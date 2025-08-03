/**
 * @file computer.hpp
 * @author zhengaohong@jisuanke.com
 * @copyright jisuanke.com
 * @version 0.0
 * @date 2016-08-01
 */

#include <math.h>
#include <stdlib.h>
#include "../include/playerbase.h"

#define min(x, y) (((x) < (y)) ? (x) : (y))

int dir[4][2] = {1, 0, -1, 0, 0, 1, 0, -1};

struct MyPoint {
    int x, y, status;
    struct MyPoint *oppo, *ghosts0, *ghosts1;
};

bool mypoint_in(struct MyPoint *p, int row_cnt, int col_cnt) {
    return 0 <= p->x && p->x < row_cnt && 0 <= p->y && p->y < col_cnt;
}

int mypoint_dis(struct MyPoint *a, struct MyPoint *b) {
    return abs(a->x - b->x) + abs(a->y - b->y);
}

int mypoint_mindist(struct MyPoint *p) {
    return min(mypoint_dis(p, p->oppo),
               min(mypoint_dis(p, p->ghosts0), mypoint_dis(p, p->ghosts1)));
}

int mypoint_cmp(const void *a, const void *b) {
    int a_min_dist = mypoint_mindist((struct MyPoint *)a);
    int b_min_dist = mypoint_mindist((struct MyPoint *)b);
    if (((MyPoint *)a)->status) {
        return a_min_dist - b_min_dist;
    } else {
        return b_min_dist - a_min_dist;
    }
}

struct Point walk(struct Player *player) {
	struct MyPoint ghost0 = {player->ghost_posx[0], player->ghost_posy[0]};
    struct MyPoint ghost1 = {player->ghost_posx[1], player->ghost_posy[1]};
    struct MyPoint oppo = {player->opponent_posx, player->opponent_posy};
    
    int status = player->your_status;
    
    struct MyPoint list[10];
    int cnt = 0;
    struct MyPoint my = {player->your_posx, player->your_posy, status, &oppo, &ghost0, &ghost1};
    list[cnt++] = my;

    for (int i = 0; i < 4; ++i) {
        int x = player->your_posx + dir[i][0];
        int y = player->your_posy + dir[i][1];

        struct MyPoint p = {x, y, status, &oppo, &ghost0, &ghost1};
        if (mypoint_in(&p, player->row_cnt, player->col_cnt) && player->mat[x][y] != '#') {
            list[cnt++] = p;
        }
    }
    qsort(list, cnt, sizeof(list[0]), mypoint_cmp);
    struct Point ret = {list[0].x, list[0].y};
    return ret;
}

void init(struct Player *player) {
}
