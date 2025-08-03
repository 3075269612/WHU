#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "../include/playerbase.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// ===================================================================
// 团队任务分配 & 函数声明
// ===================================================================

// --- 任务一：动态危险距离 (负责人: 同学A) ---
// 核心函数: isSafeMove
// 简介: 根据鬼魂的聚集程度动态调整安全距离，使AI在面对集中威胁时更加谨慎。
int isSafeMove(struct Player *player, int x, int y, int isSuper);

// --- 任务二：优化紧急避险时的目标 (负责人: 同学B) ---
// 核心函数: getSafeMove
// 简介: 在紧急避险时，优先选择朝向最近豆子的方向，使逃跑更具目的性。
struct Point getSafeMove(struct Player *player, int isSuper);

// --- 任务三：智能死胡同规避 (负责人: 同学C) ---
// 核心函数: isPathToDeadEnd, getSafeMove
// 简介: 通过前瞻性搜索来识别并评估死亡走廊，而不仅仅是单个死胡同格子。
DeadEndAnalysis isPathToDeadEnd(struct Player *player, int startX, int startY, int fromX, int fromY);

// --- 任务四：强化状态策略优化 (负责人: 同学D) ---
// 核心函数: walk
// 简介: 在超级状态下，采用性价比攻击策略，并结合“续杯”逻辑，最大化攻击收益。
struct Point walk(struct Player *player);


// ===================================================================
// 全局变量与结构体定义
// ===================================================================

// 方向数组：上、右、下、左
const int dx[4] = {-1, 0, 1, 0};
const int dy[4] = {0, 1, 0, -1};

// BFS 队列相关定义
#define QUEUE_MAX 2000
#define MAX_SIZE 60

// 死胡同分析结构体
typedef struct
{
    int is_dead_end;    // 1 如果是死胡同路径, 0 则不是
    int has_super_bean; // 1 如果路径内发现超级豆 'O'
    int total_beans;    // 路径内发现的豆子总数 ('o' 或 'O')
} DeadEndAnalysis;

typedef struct
{
	int X, Y; // 当前坐标
	int dist; // 距离起点的步数
} QueueNode;

typedef struct
{
	QueueNode data[QUEUE_MAX];
	int front;
	int rear;
} Queue;

// 队列操作函数
void initQueue(Queue *q)
{
	q->front = 0;
	q->rear = 0;
}

void enqueue(Queue *q, QueueNode node)
{
	if (q->rear < QUEUE_MAX)
	{
		q->data[q->rear++] = node;
	}
}

QueueNode dequeue(Queue *q)
{
	return q->data[q->front++];
}

int isQueueEmpty(Queue *q)
{
	return q->front == q->rear;
}

// 访问标记和父节点记录
int visited[MAX_SIZE][MAX_SIZE];
int parentX[MAX_SIZE][MAX_SIZE];
int parentY[MAX_SIZE][MAX_SIZE];

// 初始化函数（游戏开始时调用一次）
void init(struct Player *player)
{
	// 清空访问标记数组
	memset(visited, 0, sizeof(visited));
	memset(parentX, -1, sizeof(parentX));
	memset(parentY, -1, sizeof(parentY));
}

// 判断坐标是否有效且可移动
int isValid(struct Player *player, int x, int y, int isSuper)
{
	// 检查边界
	if (x < 0 || x >= player->row_cnt || y < 0 || y >= player->col_cnt)
		return 0;
	// 检查是否为墙
	if (player->mat[x][y] == '#')
		return 0;

	// 检查是否为破坏者（普通状态需要避开）
	if (!isSuper)
	{
		for (int i = 0; i < 2; i++)
		{
			if (x == player->ghost_posx[i] && y == player->ghost_posy[i])
				return 0;
		}
	}

	// 检查是否为对手（普通状态需要避开）
	if (!isSuper && player->opponent_status > 0)
	{
		if (x == player->opponent_posx && y == player->opponent_posy)
			return 0;
	}

	return 1;
}

// 检查是否是死胡同
int isDeadEnd(struct Player *player, int x, int y) {
    if (player->mat[x][y] == '#') {
        return 0; // 墙本身不是死胡同
    }

    int open_sides = 0;
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        // 检查邻居是否在地图内且不是墙
        if (nx >= 0 && nx < player->row_cnt && ny >= 0 && ny < player->col_cnt && player->mat[nx][ny] != '#') {
            open_sides++;
        }
    }

    // 如果一个格子只有一个非墙邻居，那么它就是死胡同的尽头
    return open_sides == 1;
}

// BFS 搜索最佳目标
struct Point bfsFindTarget(struct Player *player, int startX, int startY, int isSuper)
{
	Queue q;
	initQueue(&q);

	// 初始化起点
	QueueNode start = {startX, startY, 0};
	enqueue(&q, start);
	visited[startX][startY] = 1;
	parentX[startX][startY] = -1;
	parentY[startX][startY] = -1;

	// 记录找到的最佳目标
	struct Point bestTarget = {-1, -1};
	int bestScore = -1000000;

	while (!isQueueEmpty(&q))
	{
		QueueNode curr = dequeue(&q);
		int x = curr.X;
		int y = curr.Y;
		char cell = player->mat[x][y];

		// 计算当前位置得分
		int score = 0;
		int isTarget = 0;

		// 目标评分
		if (cell == 'O')
		{
			score = 1000 - curr.dist * 2; // 超级星高价值
			isTarget = 1;
		}
		else if (isSuper)
		{
			// 在强化状态下检查是否是破坏者
			for (int i = 0; i < 2; i++)
			{
				if (x == player->ghost_posx[i] && y == player->ghost_posy[i])
				{
					score = 800 - curr.dist; // 破坏者中等价值
					isTarget = 1;
					break;
				}
			}
			// 在强化状态下检查是否是普通状态的对手
			if (player->opponent_status == 0)
			{
				if (x == player->opponent_posx && y == player->opponent_posy)
				{
					score = 700 - curr.dist; // 对手中等价值
					isTarget = 1;
				}
			}
		}

		if (cell == 'o')
		{
			score = 500 - curr.dist * 3; // 普通星低价值
			isTarget = 1;
		}

		// 安全评分（普通状态）
		if (!isSuper)
		{
			int minGhostDist = 1000;
			for (int i = 0; i < 2; i++)
			{
				int dist = abs(x - player->ghost_posx[i]) + abs(y - player->ghost_posy[i]);
				if (dist < minGhostDist)
					minGhostDist = dist;
			}
			score += minGhostDist * 10; // 远离破坏者

			// 避开强化状态的对手
			if (player->opponent_status > 0)
			{
				int dist = abs(x - player->opponent_posx) + abs(y - player->opponent_posy);
				score -= (5 - dist) * 100;
			}
		}

		// 更新最佳目标
		if (isTarget && score > bestScore)
		{
			bestTarget.X = x;
			bestTarget.Y = y;
			bestScore = score;
		}

		// 尝试四个方向
		for (int i = 0; i < 4; i++)
		{
			int nx = x + dx[i];
			int ny = y + dy[i];

			if (isValid(player, nx, ny, isSuper) && !visited[nx][ny])
			{
				QueueNode next = {nx, ny, curr.dist + 1};
				enqueue(&q, next);
				visited[nx][ny] = 1;
				parentX[nx][ny] = x;
				parentY[nx][ny] = y;
			}
		}
	}

	return bestTarget;
}

// 计算从当前位置到目标位置的下一步移动
struct Point getNextStep(struct Player *player, struct Point target)
{
	int currentX = player->your_posx;
	int currentY = player->your_posy;

	// 如果没有目标，返回当前位置
	if (target.X == -1 && target.Y == -1)
	{
		struct Point p = {currentX, currentY};
		return p;
	}

	// 从目标位置回溯到起点的下一步
	int x = target.X;
	int y = target.Y;

	// 特殊处理：如果目标就是当前位置，则不需要移动
	if (x == currentX && y == currentY)
	{
		return target;
	}

	while (parentX[x][y] != -1 && parentY[x][y] != -1)
	{
		if (parentX[x][y] == currentX && parentY[x][y] == currentY)
		{
			// 找到下一步
			struct Point p = {x, y};
			return p;
		}
		// 继续回溯
		int px = parentX[x][y];
		int py = parentY[x][y];
		x = px;
		y = py;
	}

	// 如果无法回溯，返回当前位置
	struct Point p = {currentX, currentY};
	return p;
}

// 安全移动检查
int isSafeMove(struct Player *player, int x, int y, int isSuper)
{
	if (isSuper)
		return 1; // 强化状态下安全

	// 1. 动态警报距离
	int alertDistance = 1;
	int ghost_dist = abs(player->ghost_posx[0] - player->ghost_posx[1]) + abs(player->ghost_posy[0] - player->ghost_posy[1]);
	if (ghost_dist <= 4) { // 鬼魂靠得近，扩大安全距离
		alertDistance = 2;
	}


	// 检查破坏者距离
	for (int i = 0; i < 2; i++)
	{
		int dist = abs(x - player->ghost_posx[i]) + abs(y - player->ghost_posy[i]);
		if (dist <= alertDistance)
		{ // 破坏者在警报距离内，危险
			return 0;
		}
	}

	// 检查对手距离（如果对手处于强化状态）
	if (player->opponent_status > 0)
	{
		int dist = abs(x - player->opponent_posx) + abs(y - player->opponent_posy);
		if (dist <= 1)
		{ // 强化对手相邻位置危险
			return 0;
		}
	}

	return 1;
}

// 获取最近的安全移动方向
struct Point getSafeMove(struct Player *player, int isSuper)
{
	int self_x = player->your_posx;
	int self_y = player->your_posy;

	// 方向数组：上、右、下、左
	int dx[4] = {-1, 0, 1, 0};
	int dy[4] = {0, 1, 0, -1};

	// 优先尝试原定方向
	struct Point bestMove = {self_x, self_y};
	int bestScore = -1000000;

	// 包括不动
	for (int i = 0; i < 5; i++)
	{
		int nx = self_x;
		int ny = self_y;

		if (i < 4)
		{
			nx = self_x + dx[i];
			ny = self_y + dy[i];
		}

		// 检查移动是否有效
		if (nx < 0 || nx >= player->row_cnt || ny < 0 || ny >= player->col_cnt)
			continue;
		if (player->mat[nx][ny] == '#')
			continue;
		if (!isValid(player, nx, ny, isSuper))
			continue;

		// 安全评估
		int safe = isSafeMove(player, nx, ny, isSuper);
		if (!safe)
			continue;

		// 2. 优化getSafeMove的目标选择：优先吃最近的豆
		int targetScore = 0;
		int nearestBeanDist = INT_MAX;
		for (int r = 0; r < player->row_cnt; r++) {
			for (int c = 0; c < player->col_cnt; c++) {
				if (player->mat[r][c] == 'o' || player->mat[r][c] == 'O') {
					int dist = abs(nx - r) + abs(ny - c);
					if (dist < nearestBeanDist) {
						nearestBeanDist = dist;
					}
				}
			}
		}
		if (nearestBeanDist != INT_MAX) {
			targetScore = 1000 - nearestBeanDist * 10; // 距离越近，分数越高
		}


		// 安全评分
		int safetyScore = 0;
		for (int i = 0; i < 2; i++)
		{
			int dist = abs(nx - player->ghost_posx[i]) + abs(ny - player->ghost_posy[i]);
			safetyScore += dist * 10;
		}

		// 避开对手
		if (player->opponent_status > 0)
		{
			int dist = abs(nx - player->opponent_posx) + abs(ny - player->opponent_posy);
			safetyScore += dist * 5;
		}

		// 当前格子奖励
		int cellScore = 0;
		if (player->mat[nx][ny] == 'o')
			cellScore = 50;
		if (player->mat[nx][ny] == 'O')
			cellScore = 100;

        // 3. 死胡同规避 (升级版)
        DeadEndAnalysis deadEndInfo = isPathToDeadEnd(player, nx, ny, self_x, self_y);
        if (deadEndInfo.is_dead_end) {
            // 检查地图上是否还有其他豆子
            int hasOtherBeans = 0;
            for (int r = 0; r < player->row_cnt; r++) {
                for (int c = 0; c < player->col_cnt; c++) {
                    if ((player->mat[r][c] == 'o' || player->mat[r][c] == 'O') && 
                        !(r == nx && c == ny)) { // 排除当前格子
                        hasOtherBeans = 1;
                        break;
                    }
                }
                if(hasOtherBeans) break;
            }
			// 已知是死胡同
            // 更智能的死胡同评估逻辑
            if (hasOtherBeans) { // 如果地图上还有其他豆子
                if (deadEndInfo.has_super_bean) {
                    // 如果死胡同中有超级豆，给予轻微奖励，鼓励吃掉
                    safetyScore += 50;
                } else if (deadEndInfo.total_beans == 0) {
                    // 死胡同内一个豆都没有，严重惩罚
                    safetyScore -= 800;
                } else {
                    // 死胡同内有普通豆，根据豆子数量动态计算惩罚
                    // 豆子越多，惩罚越轻
                    safetyScore -= (500 - deadEndInfo.total_beans * 20);
                }
            } else {
                // 地图上只剩下死胡同里的豆子了，无论如何都要进去
                safetyScore += 100; // 反而给予奖励
            }
        }

		int totalScore = targetScore + safetyScore + cellScore;
		if (totalScore > bestScore)
		{
			bestScore = totalScore;
			bestMove.X = nx;
			bestMove.Y = ny;
		}
	}

	return bestMove;
}

// 每回合执行的移动函数
struct Point walk(struct Player *player)
{
	// 清空访问标记
	memset(visited, 0, sizeof(visited));
	memset(parentX, -1, sizeof(parentX));
	memset(parentY, -1, sizeof(parentY));

	int isSuper = (player->your_status > 0); // 检查是否处于强化状态

	// BFS 搜索最佳目标
	struct Point target = bfsFindTarget(player, player->your_posx, player->your_posy, isSuper);

	// 计算下一步移动
	struct Point nextStep = getNextStep(player, target);

	// 安全验证
	if (!isSafeMove(player, nextStep.X, nextStep.Y, isSuper))
	{
		nextStep = getSafeMove(player, isSuper);
	}

	// 4. 强化状态策略优化
	if (isSuper)
	{
		// 追击性价比最高的鬼
		struct Point bestGhostTarget = {-1, -1};
		double maxGhostScore = -1.0;

		for (int i = 0; i < 2; i++) {
			int dist = abs(player->your_posx - player->ghost_posx[i]) + abs(player->your_posy - player->ghost_posy[i]);
			if (dist > 0) {
				double score = 100.0 / dist; // 性价比：分数/距离
				if (score > maxGhostScore) {
					maxGhostScore = score;
					bestGhostTarget.X = player->ghost_posx[i];
					bestGhostTarget.Y = player->ghost_posy[i];
				}
			}
		}
		
		// 如果找到了鬼，就覆盖原目标
		if(bestGhostTarget.X != -1) {
			target = bestGhostTarget;
			nextStep = getNextStep(player, target);
		}


		// 如果强化时间快结束，优先寻找超级星以“续杯”
		if (player->your_status < 5)
		{
			// 寻找最近的超级星
			int minDist = 10000;
			struct Point superStar = {-1, -1};

			for (int i = 0; i < player->row_cnt; i++)
			{
				for (int j = 0; j < player->col_cnt; j++)
				{
					if (player->mat[i][j] == 'O')
					{
						int dist = abs(i - player->your_posx) + abs(j - player->your_posy);
						if (dist < minDist)
						{
							minDist = dist;
							superStar.X = i;
							superStar.Y = j;
						}
					}
				}
			}

			// 如果超级星可达，优先前往
			if (superStar.X != -1 && minDist < player->your_status * 2)
			{
				// 尝试直接移动
				int moveX = 0, moveY = 0;
				if (player->your_posx < superStar.X)
					moveX = 1;
				else if (player->your_posx > superStar.X)
					moveX = -1;

				if (player->your_posy < superStar.Y)
					moveY = 1;
				else if (player->your_posy > superStar.Y)
					moveY = -1;

				int nx = player->your_posx + moveX;
				int ny = player->your_posy + moveY;

				if (nx >= 0 && nx < player->row_cnt &&
					ny >= 0 && ny < player->col_cnt &&
					player->mat[nx][ny] != '#' &&
					isSafeMove(player, nx, ny, isSuper))
				{
					nextStep.X = nx;
					nextStep.Y = ny;
				}
			}
		}
	}

	return nextStep;
}

// 检查(startX, startY)是否是一个危险的死胡同的入口，并分析死胡同内的豆子情况
DeadEndAnalysis isPathToDeadEnd(struct Player *player, int startX, int startY, int fromX, int fromY) {
    // 初始化返回结构体
    DeadEndAnalysis result = {0, 0, 0};
    
    // 使用一个独立的visited数组进行本次模拟探索
    int visited_lookahead[MAX_SIZE][MAX_SIZE];
    memset(visited_lookahead, 0, sizeof(visited_lookahead));

    Queue q;
    initQueue(&q);

    // 开始探索，探索的起点是我们的"下一步"
    QueueNode startNode = {startX, startY, 0};
    enqueue(&q, startNode);
    visited_lookahead[startX][startY] = 1;
    
    // 关键：把我们现在的位置(fromX, fromY)标记为已访问，模拟"不能回头"
    visited_lookahead[fromX][fromY] = 1; 

    int path_len = 0; // 记录这条路径的长度
    int max_depth = player->row_cnt * player->col_cnt; // 探索深度的上限

    // 检查开始的格子是否有豆子
    if (player->mat[startX][startY] == 'o') {
        result.total_beans++;
    } else if (player->mat[startX][startY] == 'O') {
        result.total_beans++;
        result.has_super_bean = 1;
    }

    while(!isQueueEmpty(&q)) {
        QueueNode curr = dequeue(&q);
        path_len++;

        // 如果探索太深，可能不是一个简单的死胡同，停止以避免性能问题
        if(path_len > max_depth / 4) return result; // 不是死胡同

        // 检查当前探索到的点，是不是一个"安全的十字路口"
        int open_sides = 0;
        for (int i = 0; i < 4; i++) {
            int nx = curr.X + dx[i];
            int ny = curr.Y + dy[i];
            if (nx >= 0 && nx < player->row_cnt && ny >= 0 && ny < player->col_cnt && player->mat[nx][ny] != '#') {
                open_sides++;
            }
        }

        // 如果找到了一个有3个或更多出口的"枢纽点"，说明这条路不是死胡同，是安全的
        if (open_sides >= 3) {
            return result; // 安全，不是死胡同路径
        }

        // 继续向深处探索
        for (int i = 0; i < 4; i++) {
            int nx = curr.X + dx[i];
            int ny = curr.Y + dy[i];
            if (isValid(player, nx, ny, 0) && !visited_lookahead[nx][ny]) {
                // 检查这个格子是否有豆子
                if (player->mat[nx][ny] == 'o') {
                    result.total_beans++;
                } else if (player->mat[nx][ny] == 'O') {
                    result.total_beans++;
                    result.has_super_bean = 1;
                }
                
                visited_lookahead[nx][ny] = 1;
                QueueNode nextNode = {nx, ny, 0};
                enqueue(&q, nextNode);
            }
        }
    }

    // 如果探索完了所有可达之处，都没有找到一个"枢纽点"，那么这就是一条危险的死亡走廊
    result.is_dead_end = 1; // 确认是死胡同
    return result;
}

#endif