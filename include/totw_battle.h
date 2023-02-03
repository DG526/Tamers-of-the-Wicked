#ifndef __BATTLE_H__
#define __BATTLE_H__

#include "totw_fiend.h"
#include "totw_gui.h"


typedef enum {
	BP_Opening,
	BP_RoundPrep,
	BP_PreppingItem,
	BP_PreppingAction,
	BP_PreAction,
	BP_Acting,
	BP_PostAction,
	BP_Ending
}BattlePhase;
typedef enum {
	BS_PlayerSurpriseAttack,
	BS_FoesSurpriseAttack,
	BS_MagicUp
}BStateType;
typedef struct {
	BStateType type;
	int duration;
}BStateFlag;

BStateFlag new_battle_flag(BStateType type, int duration);

typedef struct {
	GUI* frame;
	GUI* name;
	GUI* hNum;
	GUI* hBar;
	GUI* mNum;
	GUI* mBar;
}AllyGUI;
typedef enum {
	BM_FullDialogue,
	BM_Choice_Initial,
	BM_Choice_Command,
	BM_Choice_Skill,
	BM_Choice_Item
}BattleMenu;
typedef struct {
	GUI* dialogueFrame;
	GUI* text;
	Bool scrolling;
}BattleMenu_TextDisplay;
typedef struct {
	GUI* dialogueFrame;
	GUI* dialogueText;
	GUI* optionFrame;
	GUI* opFight;
	GUI* opRecruit;
	GUI* opItem;
	GUI* opFlee;
}BattleMenu_Ch_Initial;
typedef struct {
	AllyGUI allyGUI[4];
	BattleMenu currentMenu;
	BattleMenu_TextDisplay textDisplay;
	BattleMenu_Ch_Initial choice_initial;
}BattleGUIHolder;

typedef struct Battle_S {
	Bool active;
	BattlePhase phase;		//What's happening right now?
	Entity* battlers[8];	//All of the Fiends in the battle
	int currentFiend;		//The fiend acting/accepting orders right now
	int round;				//How many full rounds have elapsed since the battle started + 1
	int accumulatedEXP;		//How much EXP the party will earn if they win. Increases every time a foe dies.
	float timeInMove;		//How many seconds have elapsed since the current phase/action began
	Skill* movePlans[8];	//List of planned moves
	Skill* moveOrder[8];	//List of planned moves organized by turn order
	BattleGUIHolder gui;	//Accessor to the battle's various menus, etc.
}Battle;
void kill_battle();
int in_battle();
void generate_new_battle(TextWord dungeon, int poolID);
void load_boss_battle(int battleID);

void battle_update();

#endif