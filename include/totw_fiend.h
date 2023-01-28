#ifndef __FIEND_H__
#define __FIEND_H__

#include "gfc_text.h"
#include "gfc_list.h"
#include "gf2d_sprite.h"
#include "totw_entity.h"

typedef enum {
	MHP = 0,
	MMP = 1,
	ATK = 2,
	DEF = 3,
	PWR = 4,
	AGL = 5
}Stat;

typedef enum {
	ST_PhysAttack,
	ST_PhysEffect,
	ST_BreathAttack,
	ST_BreathEffect,
	ST_SpellAttack,
	ST_SpellHeal,
	ST_SpellEffect
}SkillType;

typedef enum {
	Elm_Neutral,
	Elm_Fire,
	Elm_Ice,
	Elm_Electric,
	Elm_Dark
}Element;

typedef struct Skill_S {
	TextLine name;
	int requiredStats[6];
	struct Skill_S* progression;
	void (*perform)(struct Skill_S* action, int userParty, int userFiend, int targetParty, int targetFiend);
}Skill;

typedef enum {
	FT_Toughie,
	FT_Trickster,
	FT_Mage
}FiendType;

typedef struct Fiend_S {
	FiendType type;
	int size, rank;
	int HP, MP;
	int stats[6];
	float statsGrowth[6];
	List skills;
}FiendData;

void dummy_think(Entity* self);

#endif