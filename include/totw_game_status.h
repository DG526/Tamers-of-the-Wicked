#ifndef __GAME_STATUS__
#define __GAME_STATUS__

#include "gfc_types.h"

typedef enum {
	GS_Title,
	GS_Roaming,
	GS_Cutscene,
	GS_Battle,
	GS_Naming,
	GS_Managing
}GameState;

Bool game_get_quitting();
GameState game_get_state();

void game_quit();
void game_set_state(GameState status);

void game_set_resolution(int x, int y);
int game_get_resolution_x();
int game_get_resolution_y();

float game_get_wait_speed();
void game_set_wait_speed(float newSpeed);

void game_save();

#endif