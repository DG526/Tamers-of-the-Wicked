#ifndef __SKILL_H__
#define __SKILL_H__

#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "totw_entity.h"

typedef enum {
	SF_None,
	SF_Recruiting
}SkillFlag;

typedef struct Skill_S {
	Bool chosen;
	TextLine name;
	SkillFlag flag;
	int requiredStats[6];
	TextLine progression;
	TextLine original;
	int (*perform)(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
	int turnBoost;
}Skill;

Skill get_skill(TextLine skillName);

Skill skill_copy(Skill skill);

int skill_recruit_toughie(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_recruit_trickster(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_recruit_mage(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_attack(int userParty, int userFiend, int targetParty, int targetFiend, int* step);
int skill_defend(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

#endif