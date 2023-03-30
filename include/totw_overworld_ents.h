#ifndef __OVERWORLD_H__
#define __OVERWORLD_H__

#include "gfc_list.h"
#include "totw_entity.h"
#include "totw_fiend.h"
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

typedef struct {
	TextWord target;
}OWE_PortalData;

typedef struct {
	TextWord specialty;
	int difficulty;
	int money;
	int line;
	int lines;
	TextBlock dialogue[5];
	GUI* dialogueBox;
	GUI* dialogueText;
	CompassDirection direction;
}OWE_RivalData;

Entity* owe_boss_new(const char* name, Vector2D worldCoords);
void owe_boss_think(Entity* self);
void owe_boss_interact(Entity* self);

Entity* owe_portal_new(const char* target, Vector2D worldCoords, Color color);
void owe_portal_think(Entity* self);
void owe_portal_interact(Entity* self);

Entity* owe_rival_new(const char* specialty, int difficulty, Vector2D worldCoords);
void owe_rival_think(Entity* self);
void owe_rival_update(Entity* self);
void owe_rival_interact(Entity* self);

#endif