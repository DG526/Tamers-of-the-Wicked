#ifndef __NAMING_SCREEN_H__
#define __NAMING_SCREEN_H__

#include "totw_gui.h"
#include "totw_fiend.h"

typedef struct {
	Bool isNamingPlayer, isNamingFiend;
	void* namingTarget;
	TextLine* name;
	TextLine display;
	Bool acceptingInput;
	Bool capsLock;
	Bool shifted;
	GUI* nameDisplay;
	GUI* choices[39];
}NamingSystemData;

Entity* naming_system_new(FiendData* namingTarget);
Entity* naming_system_new(Entity* namingTarget);
void naming_system_think(Entity* namingSystem);
void naming_system_free(Entity* namingSystem);

int name_fiend(void* namingSystem);

#endif