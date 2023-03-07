#ifndef __GAME_STATUS__
#define __GAME_STATUS__

#include "gfc_types.h"

typedef enum {
	GS_Title,
	GS_Roaming,
	GS_Cutscene,
	GS_Battle,
	GS_Naming
}GameState;

Bool game_get_quitting();
GameState game_get_state();

void game_quit();
void game_set_state(GameState status);

void game_set_resolution(int x, int y);
int game_get_resolution_x();
int game_get_resolution_y();

void game_save();

#endif