#ifndef __BATTLE_H__
#define __BATTLE_H__

#include "totw_fiend.h"
#include "totw_gui.h"


typedef enum {
	BP_Opening,
	BP_RoundPrep,
	BP_PreppingItem,
	BP_PreppingAction,
	BP_PreppingRecruit,
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
	GUI* dialogueFrame;
	GUI* dialogueText;
	GUI* options[4];
}BattleMenu_Targeting;
typedef struct {
	AllyGUI allyGUI[4];
	BattleMenu currentMenu;
	BattleMenu_TextDisplay textDisplay;
	BattleMenu_Ch_Initial choice_initial;
	BattleMenu_Targeting target_ally, target_enemy;
}BattleGUIHolder;

typedef struct Battle_S {
	Bool active;
	BattlePhase phase;		//What's happening right now?
	Entity* battlers[8];	//All of the Fiends in the battle
	int currentFiend;		//The fiend acting/accepting orders right now
	int round;				//How many full rounds have elapsed since the battle started + 1
	int accumulatedEXP;		//How much EXP the party will earn if they win. Increases every time a foe dies.
	int turn;
	int turnPhase;
	int timeForMove;		//The time at which action will continue.
	Skill movePlans[8];		//List of planned moves
	int moveOrderDeterminer[8];	//List of ints to sort from highest->lowest
	Vector4D movePlanUT[8];	//List of user/target identifiers for plans, x = user party, y = user member, z = target party, w = target member
	Skill moveOrder[8];		//List of planned moves organized by turn order
	Vector4D moveOrderUT[8];	//List of user/target identifiers for actions in order, x = user party, y = user member, z = target party, w = target member
	BattleGUIHolder gui;	//Accessor to the battle's various menus, etc.

	Bool canRecruit;
	Bool recruiting;
	Bool recruited;
	int recruitTarget;
	float recruitChance;
	int recruitAttempt;

	Bool victorious;
}Battle;
void kill_battle();
int in_battle();
void battle_wait(int milliseconds);
void generate_new_battle(TextWord dungeon, int poolID);
void load_boss_battle(int battleID);

void battle_update();

FiendData* battle_get_party_member(int party, int member);
void battle_set_main_dialogue(TextBlock text);

int startRound_fight(void* target);
int startRound_recruit(void* target);
void battle_impress_enemy(float percent);
float battle_get_recruit_chance();

void battle_add_exp(int exp);

#endif