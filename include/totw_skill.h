#ifndef __SKILL_H__
#define __SKILL_H__

#include "gfc_text.h"
#include "gfc_audio.h"
#include "gf2d_sprite.h"
#include "totw_entity.h"

typedef enum {
	SkS_Ready_Attack,
	SkS_Ready_Attack_Special,
	SkS_Ready_Magic_Pos,
	SkS_Ready_Magic_Neg,
	SkS_Hit,
	SkS_Kill,
	SkS_BossKill
} SkillSound;

void battle_sounds_init();
void battle_sounds_free();

typedef enum {
	SF_None,
	SF_Recruiting,
	SF_Spell
}SkillFlag;

typedef enum {
	STT_Enemy_1,
	STT_Enemy_All,
	STT_Ally_1,
	STT_Ally_All,
	STT_Ally_Corpse,
	STT_Self,
	STT_All
}SkillTargetingType;

typedef struct Skill_S {
	Bool inUse;

	SkillTargetingType targetingType;
	Bool chosen;
	TextLine name;
	TextLine description;
	SkillFlag flag;
	int requiredStats[6];
	TextLine progression;
	TextLine original;
	int (*perform)(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
	int manaCost;
	int turnBoost;
	int tacticFavor[3];
}Skill;

Skill get_skill(TextLine skillName);

Skill skill_copy(Skill skill);

int skill_recruit_toughie(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_recruit_trickster(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_recruit_mage(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_attack(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_defend(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_flamebeam(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_flameburst(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_flamebane(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_snowbeam(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_snowburst(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_snowbane(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_fulgurbeam(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_fulgurburst(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_fulgurbane(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_umbrabeam(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_umbraburst(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_umbrabane(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_flameboom(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_flamedokaan(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_snowboom(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_snowdokaan(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_fulgurboom(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_fulgurdokaan(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_umbraboom(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_umbradokaan(int userParty, int userFiend, int targetParty, int targetFiend, int* step);


int skill_sparkbreath(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_firegale(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_chillbreath(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_frostgale(int userParty, int userFiend, int targetParty, int targetFiend, int* step);


int skill_fasthit(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_powerhit(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

#endif