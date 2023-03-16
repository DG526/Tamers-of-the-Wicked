#ifndef __FIEND_H__
#define __FIEND_H__

#include "simple_json.h";
#include "gfc_text.h"
#include "gfc_list.h"
#include "gf2d_sprite.h"
#include "totw_entity.h"
#include "totw_skill.h"

typedef enum {
	MHP = 0,
	MMP = 1,
	ATK = 2,
	DEF = 3,
	PWR = 4,
	AGL = 5
}Stat;

typedef enum {
	FS_Null = 0,
	FS_Defending,
	FS_Stun,
	FS_Poison,
	FS_Blind,
	FS_CounterReady
}FiendStatusType;
typedef struct {
	int enabled;
	FiendStatusType status;
	int duration;
}FiendStatus;

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

typedef enum {
	Destruction,	//Focus on doing as much damage as possible
	Chaos,			//Focus on applying status effects
	Support,		//Focus on healing and buffs
	Magicless		//Don't use MP
}Tactic;

typedef enum {
	FndT_Toughie,	//Fiends that specialize in brute force, usually have high HP, ATK, & DEF.
	FndT_Trickster,	//Fiends that are unpredictable all-rounders, usually have high MP & AGL.
	FndT_Mage		//Fiends that specialize in magic, usually have high MP & PWR.
}FiendType;

typedef struct Fiend_S {
	Entity* entity;
	FiendType type;			//class
	TextLine species;		//species name
	TextLine name;			//given name, this will be read in battle
	int party;				//1 for player, 2 for opponent
	Sprite* sprite;			//Main sprite, shown in battle for foes.
	int size;				//1-3, how many slots the fiend takes up (out of 4)
	int rank;				//1-5, measure of general power
	int level;				//1-50, increases with EXP
	int exp;				//For allies, allows for levelling up. For enemies, shows how much given upon death.
	int HP, MP;				//Current variable stat pool
	int stats[6];			//Base stats, use Stat enum to get/set particular stat.
	int statSpellBonuses[6];	//Stat modifiers mid-combat, use Stat enum to get/set particular stat.
	int statBreedBonuses[6];	//Stat modifiers post-fusion, use Stat enum to get/set particular stat.
	int statsMin[6];		//Base stats at Lv. 1, use Stat enum to get/set particular stat.
	int statsMax[6];		//Base stats at Lv. 50, use Stat enum to get/set particular stat.
	Skill skills[10];		//List of skills learned.
	Skill potSkills[19];	//List of unlearned potential skills.
	Skill selectedSkill;
	Vector2D skillTarget;
	Tactic tactic;			//Determines what moves AI chooses.
	FiendStatus statuses[10];			//Status effects.
}FiendData;

Bool fiend_has_status(FiendData* fiend, FiendStatusType status);
void fiend_remove_status(FiendData* fiend, FiendStatusType status);
void fiend_apply_status(FiendData* fiend, FiendStatusType status, int duration);

typedef struct {
	FiendData* fiends[100];
	int fiendCount;
}Menagerie;
typedef struct {
	TextLine fiendNames[4];
	FiendData* fiends[4];
	int unitCount;
	int slotsTaken;
}Party;

void ally_list_load();
Bool ally_list_is_loaded();
void party_load();
void party_close();
int party_get_member_count();
int menagerie_get_member_count();
Entity* party_read_member(int member);
FiendData* party_read_member_data(int member);
FiendData* menagerie_read_member_data_by_index(int index);
SJson* menagerie_get_as_json();
FiendData* read_fiend(const char* species);

void dummy_think(Entity* self);

void calculate_stats(FiendData* tgt);
int calculate_stat(int minimum, int maximum, int level);

int fiend_damage(FiendData* target, int raw);
float fiend_impress(FiendData* target, float raw);

Bool fiend_name_taken(TextLine query);

void fiend_register_new(FiendData* newFiend);
void fiend_part_ways(TextLine name);

void party_add_fiend(FiendData* newFiend);
void party_remove_fiend(TextLine name);

int fiend_check_level_up(FiendData* self, TextBlock* textDisplay, int* step, Bool* leveledUp);

#endif