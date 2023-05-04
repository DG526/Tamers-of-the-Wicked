#ifndef __FIEND_MANAGEMENT_H__
#define __FIEND_MANAGEMENT_H__

#include "totw_fiend.h"
#include "totw_gui.h"

typedef struct {
	GUI* panels[5];
	GUI* leftSprite, *rightSprite;
	GUI* slotDisplay;		// "Slots: n/4"
	GUI* partyOptions[5];	// First four are party slots, last is confirmation.
	GUI* message;
	GUI* menagerieList;
	int fiendToReplace;
	Bool replacing;
	GameState returnState;
}PartyManagerData;

Entity* party_manager_new();
void party_manager_think(Entity* partyManager);
void party_manager_free(Entity* partyManager);

#endif