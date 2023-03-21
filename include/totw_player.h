#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "totw_entity.h"

typedef enum {
	PS_Inactive = -1,
	PS_Idle = 0,
	PS_Walking,
	PS_Stopping
}PlayerState;

typedef struct {
	PlayerState state;
	int money;
	Bool focused; //Whether the camera is focusing on the player
	CompassDirection direction;
	int battleSteps;
}PlayerData;

Entity* player_new();
Entity* player_get();

void player_think(Entity* self);
void player_update(Entity* self);

void player_set_coords(Vector2D worldCoords);
void player_set_dir(CompassDirection dir);

#endif