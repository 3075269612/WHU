#include <math.h>
#include <stdlib.h>
#include "../include/playerbase.h"

struct Point walk(struct Player *player) {
	struct Point ret = {player->your_posx, player->your_posy};
    return ret;
}

void init(struct Player *player) {
}
