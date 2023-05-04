#ifndef __ITEM_H__
#define __ITEM_H__

#include "totw_fiend.h"

typedef enum {
	ITT_Ally,
	ITT_DeadAlly,
	ITT_Enemy,
	ITT_None
}ItemTargetingType;

typedef enum {
	Item_FirstAid,
	Item_MagicDew,
	Item_Soulstim,
	Item_SpiritSpirits,
	Item_BitterMedicine,
	Item_PortalSigil,
	Item_MintPerfume,
	Item_LemonPerfume,
	Item_SacredDagger,
	Item_SacredSword
}ItemType;

typedef struct Item_S{
	ItemType type;
	ItemTargetingType targetingType;
	TextWord name;
	TextLine obtainLine, useUpLine, description;
	int (*use)(struct Item_S* item, FiendData* target, int* step); //If targeting type is None, pass NULL.
	int useTime; //1st bit is battle, 2nd is field.
	float consumeChance; // 1.0 = always consumed, 0.5 = 50% chance of being used up, etc.
	int buyCost; // Sell cost is half of buy cost, if cost is 0, cannot be sold.
}Item;

typedef struct {
	List* items;
	int funds;
}Inventory;

void inventory_load();

void inventory_add_item(ItemType type);
Item* inventory_get_item(int index);
void inventory_remove_item(int index);
void inventory_remove_item_by_ref(Item* item);
List* inventory_get_items();
int inventory_get_funds();
void inventory_modify_funds(int change); //Adds or subtracts funds;

#endif