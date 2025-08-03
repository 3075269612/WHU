/**
 * @file player.h
 * @author yangboyang@jisuanke.com
 * @copyright jisuanke.com
 * @date 2021-07-01
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/playerbase.h"

int data[50][50], n, m;
int dis[50][50][50][50];
int step[4][2] = {1, 0, -1, 0, 0, 1, 0, -1};
int vst[50][50], use[50][50];

int in(int x, int y) {
    return x >= 0 && x < n && y >= 0 && y < m && data[x][y] != -1;
}

void init(struct Player *player) {
    n = player->row_cnt;
    m = player->col_cnt;
    memset(use, 0, sizeof(use));
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (player->mat[i][j] == '#') data[i][j] = -1;
            else if (player->mat[i][j] == '.') data[i][j] = 0;
            else if (player->mat[i][j] == 'o') data[i][j] = 1;
            else data[i][j] = 2;
        }
    }
	// This function will be executed at the begin of each game, only once.
}

int same(int ax, int ay, int bx, int by) {
    return ax == bx && ay == by;
}

int ok(int x, int y, struct Player *player) {
    if (player->your_status) return 1;
    for (int i = 0; i < 2; i++) {
        if (same(x, y, player->ghost_posx[i], player->ghost_posy[i])) {
            return 0;
        }
        for (int k = 0; k < 4; k++) {
            int dx = x + step[k][0], dy = y + step[k][1];
            if (in(dx, dy) && same(dx, dy, player->ghost_posx[i], player->ghost_posy[i])) {
                return 0;
            }
        }
    }
    return 1;
}

struct Point walk(struct Player *player) {
    use[player->your_posx][player->your_posy] = 1;
    int x = player->your_posx, y = player->your_posy;
    int ansx = -1, ansy = -1, ans = 1e9, dir, tot = 0;
    while (true) {
        dir = rand() % 4;
        tot++;
        if (tot >= 100) break;
        int dx = x + step[dir][0];
        int dy = y + step[dir][1];
        if (in(dx, dy) && ok(dx, dy, player)) {
            use[dx][dy] = 1;
            struct Point ret = {dx, dy};
            return ret;
        }
    }
	struct Point ret = {player->your_posx, player->your_posy};
    return ret;
}
