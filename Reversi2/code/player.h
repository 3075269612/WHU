#define INF -11451428
#define MAX 11451428
#define SEARCH_WIDTH 5 // 搜索宽度，控制每层只保留前5优先点
#define SEARCH_DEPTH 6 // 搜索深度

#include <string.h>
#include "../include/playerbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// 八个方向向量，便于遍历棋盘方向
int directions[8][2] = { 0, 1, 0, -1, 1, 0, -1, 0, 1, 1, -1, -1, 1, -1, -1, 1 };

// 棋盘和权值表
char board[13][13];         // 当前棋盘
int weight[13][13];         // 每个格子的权值
int best_x, best_y;         // 当前搜索到的最优落子点

// 参数
int mobility_weight;        // 行动力权重
int endgame_weight;         // 终局权重
int piece_count;            // 棋子总数

// 函数声明
// 计算己方稳定子数量（角、边、内部），用于评估局面稳定性
int getStableDiscs(struct Player *player, char board[13][13]);

// 判断(x, y)位置在当前棋盘上是否为合法落子点 ****
int isValidMove(struct Player *player, int x, int y, char board[13][13]);

// 初始化棋盘和权值表，准备AI搜索
void init(struct Player *player);

// 选择当前局面下的最佳落子点（主入口）
struct Point place(struct Player *player);

// 统计棋盘上己方棋子数量
int countDiscs(struct Player *player, char board[13][13]);

// 极大极小搜索+剪枝，递归搜索最优解
int dfs(struct Player *player, int step, char my_board[13][13], char opp_board[13][13], int alpha_beta);

// 设置指定角落及其周围格子的权值
void setCornerWeights(int x, int y);

// 在(x, y)处落子，并翻转被夹住的对方棋子 ****
void applyMove(struct Player *player, int x, int y, char my_board[13][13], char opp_board[13][13]);

// 复制棋盘src到dest
void copyBoard(int n, char dest[13][13], char src[13][13]);

// 计算当前棋盘局面对己方的权值
int getBoardWeight(struct Player *player, char board[13][13]);

// 计算己方与对方的行动力（可落子数）差值
int getMobility(struct Player *player, char my_board[13][13], char opp_board[13][13]);

// 计算棋盘上己方棋子数量
int countDiscs(struct Player* player, char board[13][13]) {
    int count = 0;
    for (int i = 0; i < player->row_cnt; i++) {
        for (int j = 0; j < player->col_cnt; j++) {
            if (board[i][j] == 'O' || board[i][j] == 'o') {
                count++;
            }
        }
    }
    return count;
}

// 计算稳定子数量（角、边等）
// 返回己方棋盘上不会再被翻转的棋子数量
int getStableDiscs(struct Player* player, char board[13][13]) {
    int stable[3] = { 0, 0, 0 }; // stable[0]:角落, stable[1]:边, stable[2]:内部
    int cind1[4] = { 0 };
    int cind2[4] = { 0 }; // 四个角的行列索引
    cind1[2] = cind1[3] = player->col_cnt - 1;
    cind2[1] = cind2[2] = player->col_cnt - 1;
    int inc1[4] = { 0, 1, 0, -1 }; // 行方向增量
    int inc2[4] = { 1, 0, -1, 0 }; // 列方向增量
    int stop[4] = { 0 };
    int i, j;
    // 检查四个角及其延伸方向上的稳定子
    for (i = 0; i < 4; i++)
    {
        // 如果角落有己方棋子
        if (board[cind1[i]][cind2[i]] == 'O')
        {
            stop[i] = 1;
            stable[0] += 1; // 角落稳定子+1
            // 沿着边界方向继续查找稳定子
            for (j = 1; j < player->col_cnt; j++)
            {
                if (board[cind1[i] + inc1[i] * j][cind2[i] + inc2[i] * j] != 'O')
                {
                    break;
                }
                else
                {
                    stop[i] = j + 1;
                    stable[1] += 1; // 边稳定子+1
                }
            }
        }
    }
    // 检查边界反方向上的稳定子
    for (i = 0; i < 4; i++)
    {
        if (board[cind1[i]][cind2[i]] == 'O')
        {
            for (j = 1; j < player->col_cnt - stop[(i + 3) % 4]; j++)
            {
                if (board[cind1[i] - inc1[(i + 3) % 4] * j][cind2[i] - inc2[(i + 3) % 4] * j] != 'O')
                {
                    break;
                }
                else
                {
                    stable[1] += 1;
                }
            }
        }
    }
    // 检查整行、整列、对角线是否全为己方棋子
    int colfull[13] = { 0 };
    int rowfull[13] = { 0 };
    int diag1full[26] = { 0 };
    int diag2full[26] = { 0 };
    for (i = 0; i < player->col_cnt; i++)
    {
        // 检查第i行是否全为己方棋子
        for (j = 0; j < player->col_cnt; j++)
        {
            if (board[i][j] != 'O' && board[i][j] != 'o')
                break;
        }
        if (j == player->col_cnt)
            rowfull[i] = 1;
        // 检查第i列是否全为己方棋子
        for (j = 0; j < player->col_cnt; j++)
        {
            if (board[j][i] != 'O' && board[j][i] != 'o')
                break;
        }
        if (j == player->col_cnt)
            colfull[i] = 1;
    }
    // 检查所有对角线是否全为己方棋子
    for (i = 0; i < player->col_cnt * 2 - 1; i++)
    {
        int diacnt = player->col_cnt - abs((player->col_cnt * 2 - 2) / 2 - i);
        int startx, starty;
        // 主对角线
        if (i < player->col_cnt - 1)
        {
            startx = player->col_cnt - 1 - i;
            starty = 0;
        }
        else
        {
            startx = 0;
            starty = i - player->col_cnt + 1;
        }
        for (j = 0; j < diacnt; j++)
        {
            if (board[startx + j][starty + j] != 'O' && board[startx + j][starty + j] != 'o')
                break;
        }
        if (j == diacnt)
            diag1full[i] = 1;
        // 副对角线
        if (i < player->col_cnt - 1)
        {
            startx = player->col_cnt - 1 - i;
            starty = player->col_cnt - 1;
        }
        else
        {
            startx = 0;
            starty = 2 * player->col_cnt - 2 - i;
        }
        for (j = 0; j < diacnt; j++)
        {
            if (board[startx + j][starty - j] != 'O' && board[startx + j][starty - j] != 'o')
                break;
        }
        if (j == diacnt)
            diag2full[i] = 1;
    }
    // 检查内部稳定子（四个方向都被己方占满才算）
    for (i = 1; i < player->col_cnt - 1; i++)
    {
        for (j = 1; j < player->col_cnt - 1; j++)
        {
            int diag1 = j - i + player->col_cnt - 1;
            int diag2 = 2 * player->col_cnt - 2 - j - i;
            if (board[i][j] == 'O' && colfull[j] && rowfull[i] && diag1full[diag1] && diag2full[diag2])
            {
                stable[2]++;
            }
        }
    }
    // 返回所有稳定子数量
    return stable[0] + stable[1] + stable[2];
}

// 落子并翻转棋子
// 在(x, y)处落子，并将被夹住的对方棋子翻转为己方
void applyMove(struct Player* player, int x, int y, char my_board[13][13], char opp_board[13][13]) {
    my_board[x][y] = 'O';
    opp_board[x][y] = 'o';
    // 检查8个方向
    for (int dir = 0; dir < 8; dir++)
    {
        int nx = x + directions[dir][0];
        int ny = y + directions[dir][1];
        if (nx < 0 || nx >= player->row_cnt || ny < 0 || ny >= player->col_cnt)
        {
            continue;
        }
        if (my_board[nx][ny] != 'o')
        {
            continue;
        }
        int x0 = nx, y0 = ny;
        // 沿该方向查找是否能夹住对方棋子
        while (true)
        {
            nx += directions[dir][0];
            ny += directions[dir][1];
            if (nx < 0 || nx >= player->row_cnt || ny < 0 || ny >= player->col_cnt || (my_board[nx][ny] >= '1' && my_board[nx][ny] <= '9'))
            {
                break;
            }
            if (my_board[nx][ny] == 'O')
            {
                // 夹住后，反向填充己方棋子
                for (int i = x0, j = y0; i != nx || j != ny; i += directions[dir][0], j += directions[dir][1])
                {
                    my_board[i][j] = 'O';
                    opp_board[i][j] = 'o';
                }
                break;
            }
        }
    }
}

// 复制棋盘
// 将src棋盘复制到dest
void copyBoard(int n, char dest[13][13], char src[13][13]) {
    for (int i = 0; i < n; i++)
        strcpy(dest[i], src[i]);
}

// 计算行动力差值
// 返回己方可落子数-对方可落子数
int getMobility(struct Player* player, char my_board[13][13], char opp_board[13][13]) {
    int my_moves = 0, opp_moves = 0;
    for (int i = 0; i < player->row_cnt; i++) {
        for (int j = 0; j < player->col_cnt; j++) {
            if (isValidMove(player, i, j, my_board)) my_moves++;
            if (isValidMove(player, i, j, opp_board)) opp_moves++;
        }
    }
    return my_moves - opp_moves;
}

// 计算当前局面权值
// 只统计己方棋子权重总和，若一方无棋子直接返回极值
int getBoardWeight(struct Player* player, char board[13][13]) {
    int total_weight = 0, my_count = 0, opp_count = 0;
    piece_count = 0;
    for (int i = 0; i < player->row_cnt; i++) {
        for (int j = 0; j < player->col_cnt; j++) {
            if (board[i][j] == 'O') {
                total_weight += weight[i][j];
                my_count++;
                piece_count++;
            } else if (board[i][j] == 'o') {
                opp_count++;
            }
        }
    }
    if (my_count == 0) return -100000;
    if (opp_count == 0) return 1000000;
    return total_weight;
}

// 搜索主函数，极大极小搜索+剪枝
int dfs(struct Player* player, int step, char my_board[13][13], char opp_board[13][13], int alpha_beta) {
    // 搜索到最大深度，直接评估局面
    if (step > SEARCH_DEPTH)
    {
        return getBoardWeight(player, my_board) + mobility_weight * getMobility(player, my_board, opp_board) + 10 * getStableDiscs(player, my_board) + endgame_weight * piece_count;
    }
    int x[SEARCH_WIDTH + 1], y[SEARCH_WIDTH + 1];
    for (int i = 0; i < SEARCH_WIDTH; i++)
    {
        x[i] = y[i] = -1;
    }
    // 只保留权值最高的SEARCH_WIDTH个点进行扩展
    for (int i = 0; i < player->row_cnt; i++)
    {
        for (int j = 0; j < player->row_cnt; j++)
        {
            if (isValidMove(player, i, j, my_board))
            {
                int k = SEARCH_WIDTH - 1;
                while (k >= 0 && (x[k] == -1 || weight[i][j] > weight[x[k]][y[k]]))
                {
                    x[k + 1] = x[k];
                    y[k + 1] = y[k];
                    k--;
                }
                x[k + 1] = i;
                y[k + 1] = j;
            }
        }
    }
    // 无法落子，交换双方身份递归
    if (x[0] == -1)
    {
        if (step == 1)
        {
            best_x = -1, best_y = -1;
            return 0;
        }
        char next_mat[13][13];
        char n_o_mat[13][13];
        copyBoard(player->row_cnt, next_mat, my_board);
        copyBoard(player->row_cnt, n_o_mat, opp_board);
        if (step % 2 == 1)
        {
            return dfs(player, step + 1, n_o_mat, next_mat, INF);
        }
        else
        {
            return dfs(player, step + 1, n_o_mat, next_mat, MAX);
        }
    }
    int ex_value;
    if (step % 2 == 1)
    {
        ex_value = INF;
    }
    else
    {
        ex_value = MAX;
    }
    // 极大极小搜索+剪枝
    for (int i = 0; i < SEARCH_WIDTH && x[i] != -1; i++)
    {
        char next_mat[13][13];
        char n_o_mat[13][13];
        copyBoard(player->row_cnt, next_mat, my_board);
        copyBoard(player->row_cnt, n_o_mat, opp_board);
        applyMove(player, x[i], y[i], next_mat, n_o_mat);
        int value = dfs(player, step + 1, n_o_mat, next_mat, ex_value);
        if (step % 2 == 1)
        {
            if (value > alpha_beta)
            {
                return MAX; // 剪枝
            }
            if (value > ex_value)
            {
                ex_value = value;
                if (step == 1)
                {
                    best_x = x[i];
                    best_y = y[i];
                }
            }
        }
        else
        {
            if (value < alpha_beta)
            {
                return INF; // 剪枝
            }
            if (value < ex_value)
            {
                ex_value = value;
            }
        }
    }
    return ex_value;
}

// 判断是否合法落子
int isValidMove(struct Player* player, int x, int y, char board[13][13]) {
    if (x < 0 || x >= player->row_cnt || y < 0 || y >= player->col_cnt)
    {
        return false;
    }
    if (board[x][y] == 'o' || board[x][y] == 'O')
    {
        return false;
    }
    // 检查8个方向是否能夹住对方棋子
    for (int dir = 0; dir < 8; dir++)
    {
        int nx = x + directions[dir][0];
        int ny = y + directions[dir][1];
        if (nx < 0 || nx >= player->row_cnt || ny < 0 || ny >= player->col_cnt)
        {
            continue;
        }
        if (board[nx][ny] != 'o')
        {
            continue;
        }
        while (true)
        {
            nx += directions[dir][0];
            ny += directions[dir][1];
            if (nx < 0 || nx >= player->row_cnt || ny < 0 || ny >= player->col_cnt || (board[nx][ny] >= '1' && board[nx][ny] <= '9'))
            {
                break;
            }
            if (board[nx][ny] == 'O')
            {
                return true;
            }
        }
    }
    return false;
}

// 设置角落及其周围权值（角落周围格子权值较低，角落本身权值极高）
void setCornerWeights(int x, int y) {
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            weight[x + i][y + j] = -25;
}

// 初始化棋盘和权值表
void init(struct Player* player) {
    // 复制棋盘
    for (int i = 0; i < player->row_cnt; i++)
        for (int j = 0; j < player->col_cnt; j++)
            board[i][j] = player->mat[i][j];
    int row = player->row_cnt;
    int col = player->col_cnt;
    // 初始化权值表
    for (int i = 0; i < row; i++)
        for (int j = 0; j < col; j++)
            weight[i][j] = 2;
    // 设置四个角落及其周围的权值
    setCornerWeights(0, 0);
    setCornerWeights(0, col - 2);
    setCornerWeights(row - 2, 0);
    setCornerWeights(row - 2, col - 2);
    weight[0][0] = weight[0][col - 1] = weight[row - 1][0] = weight[row - 1][col - 1] = 500;
    weight[1][1] = weight[1][col - 2] = weight[row - 2][1] = weight[row - 2][col - 2] = -45;
    weight[2][2] = weight[2][col - 3] = weight[row - 3][2] = weight[row - 3][col - 3] = 3;
    for (int i = 2; i < col - 2; i++)
    {
        weight[0][i] = weight[row - 1][i] = 5;
        weight[1][i] = weight[row - 2][i] = 1;
    }
    for (int i = 2; i < row - 2; i++)
    {
        weight[i][0] = weight[i][col - 1] = 5;
        weight[i][1] = weight[i][col - 2] = 1;
    }
    weight[0][2] = weight[0][col - 3] = weight[row - 1][2] = weight[row - 1][col - 3] = 10;
    weight[2][0] = weight[row - 3][0] = weight[2][col - 1] = weight[row - 3][col - 1] = 10;
}

// 选择最佳落点（主入口，返回当前最优落子点）
struct Point place(struct Player* player) {
    char opp_board[13][13], my_board[13][13];
    // 根据棋盘大小设置行动力权重
    if (player->col_cnt == 8)
    {
        mobility_weight = 15;
    }
    else if (player->col_cnt == 10)
    {
        mobility_weight = 12;
    }
    else
    {
        mobility_weight = 10;
    }
    // 复制棋盘并交换己方/对方棋子符号
    for (int i = 0; i < player->row_cnt; i++)
        for (int j = 0; j < player->col_cnt; j++)
        {
            my_board[i][j] = player->mat[i][j];
            if (player->mat[i][j] == 'O')
            {
                opp_board[i][j] = 'o';
            }
            else if (player->mat[i][j] == 'o')
            {
                opp_board[i][j] = 'O';
            }
            else
            {
                opp_board[i][j] = player->mat[i][j];
            }
        }
    // 统计当前棋盘棋子数
    int chess = countDiscs(player, my_board);
    // 终局时加大终局权重
    if (player->row_cnt * player->row_cnt - chess <= player->row_cnt)
    {
        endgame_weight = 2;
    }
    else
    {
        endgame_weight = 0;
    }
    // 搜索最优解
    dfs(player, 1, my_board, opp_board, MAX);
    struct Point best_point = initPoint(best_x, best_y);
    return best_point;
}
