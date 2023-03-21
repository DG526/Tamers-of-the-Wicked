#ifndef __OVERWORLD_H__
#define __OVERWORLD_H__

#include "gfc_list.h"
#include "totw_entity.h"
#include "totw_gui.h"

typedef struct {
	Bool started;
	int line;
	int lines;
	TextBlock dialogue[16];
	TextWord name;
	GUI* dialogueBox;
	GUI* dialogueText;
	CompassDirection direction;
}OWE_BossData;

Entity* owe_boss_new(const char* name, Vector2D worldCoords);
void owe_boss_think(Entity* self);
void owe_boss_interact(Entity* self);

#endif