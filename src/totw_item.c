#include "simple_logger.h"

#include "totw_item.h"
#include "totw_player.h"
#include "totw_battle.h"

static Inventory inventory;
static Item items[10];

int item_use_firstAid(Item* self, FiendData* target, int* step);
int item_use_magicDew(Item* self, FiendData* target, int* step);
int item_use_soulstim(Item* self, FiendData* target, int* step);
int item_use_spiritSpirits(Item* self, FiendData* target, int* step);
int item_use_bitterMedicine(Item* self, FiendData* target, int* step);
int item_use_portalSigil(Item* self, FiendData* target, int* step);
int item_use_mintPerfume(Item* self, FiendData* target, int* step);
int item_use_lemonPerfume(Item* self, FiendData* target, int* step);
int item_use_sacredDagger(Item* self, FiendData* target, int* step);
int item_use_sacredSword(Item* self, FiendData* target, int* step);

void inventory_init(int maxItems) {
	inventory.items = gfc_list_new_size(maxItems);

	items[Item_FirstAid].buyCost = 15;
	items[Item_FirstAid].consumeChance = 1;
	items[Item_FirstAid].targetingType = ITT_Ally;
	items[Item_FirstAid].useTime = 3;
	items[Item_FirstAid].type = Item_FirstAid;
	items[Item_FirstAid].use = item_use_firstAid;
	gfc_word_cpy(items[Item_FirstAid].name, "First-Aid Kit");
	gfc_line_cpy(items[Item_FirstAid].description, "Recovers some HP.");
	gfc_line_cpy(items[Item_FirstAid].obtainLine, "You find a First-Aid Kit.");

	items[Item_MagicDew].buyCost = 50;
	items[Item_MagicDew].consumeChance = 1;
	items[Item_MagicDew].targetingType = ITT_Ally;
	items[Item_MagicDew].useTime = 3;
	items[Item_MagicDew].type = Item_MagicDew;
	items[Item_MagicDew].use = item_use_magicDew;
	gfc_word_cpy(items[Item_MagicDew].name, "Magic Dew");
	gfc_line_cpy(items[Item_MagicDew].description, "Recovers some MP.");
	gfc_line_cpy(items[Item_MagicDew].obtainLine, "You find a phial of Magic Dew.");

	items[Item_Soulstim].buyCost = 3000;
	items[Item_Soulstim].consumeChance = 1;
	items[Item_Soulstim].targetingType = ITT_DeadAlly;
	items[Item_Soulstim].useTime = 3;
	items[Item_Soulstim].type = Item_Soulstim;
	items[Item_Soulstim].use = item_use_soulstim;
	gfc_word_cpy(items[Item_Soulstim].name, "Soulstim");
	gfc_line_cpy(items[Item_Soulstim].description, "Brings the dead back to life.");
	gfc_line_cpy(items[Item_Soulstim].obtainLine, "You find a flask of Soulstim!");

	items[Item_SpiritSpirits].buyCost = 200;
	items[Item_SpiritSpirits].consumeChance = 1;
	items[Item_SpiritSpirits].targetingType = ITT_Ally;
	items[Item_SpiritSpirits].useTime = 3;
	items[Item_SpiritSpirits].type = Item_SpiritSpirits;
	//items[Item_SpiritSpirits].use = item_use_spiritSpirits;
	gfc_word_cpy(items[Item_SpiritSpirits].name, "Spirit Spirits");
	gfc_line_cpy(items[Item_SpiritSpirits].description, "A beverage that exchanges HP for MP.");
	gfc_line_cpy(items[Item_SpiritSpirits].obtainLine, "You find a bottle of Spirit Spirits.");

	items[Item_BitterMedicine].buyCost = 250;
	items[Item_BitterMedicine].consumeChance = 1;
	items[Item_BitterMedicine].targetingType = ITT_Ally;
	items[Item_BitterMedicine].useTime = 3;
	items[Item_BitterMedicine].type = Item_BitterMedicine;
	//items[Item_BitterMedicine].use = item_use_bitterMedicine;
	gfc_word_cpy(items[Item_BitterMedicine].name, "Bitter Medicine");
	gfc_line_cpy(items[Item_BitterMedicine].description, "A medicine that exchanges MP for HP.");
	gfc_line_cpy(items[Item_BitterMedicine].obtainLine, "You find a Bitter Medicine.");

	items[Item_PortalSigil].buyCost = 25;
	items[Item_PortalSigil].consumeChance = 0.5;
	items[Item_PortalSigil].targetingType = ITT_None;
	items[Item_PortalSigil].useTime = 2;
	items[Item_PortalSigil].type = Item_PortalSigil;
	//items[Item_PortalSigil].use = item_use_portalSigil;
	gfc_word_cpy(items[Item_PortalSigil].name, "Portal Sigil");
	gfc_line_cpy(items[Item_PortalSigil].description, "Opens a portal to go home.");
	gfc_line_cpy(items[Item_PortalSigil].obtainLine, "You find a page with a Portal Sigil.");
	gfc_line_cpy(items[Item_PortalSigil].useUpLine, "The Portal Sigil vanished from the page.");

	items[Item_MintPerfume].buyCost = 150;
	items[Item_MintPerfume].consumeChance = 0.6;
	items[Item_MintPerfume].targetingType = ITT_None;
	items[Item_MintPerfume].useTime = 2;
	items[Item_MintPerfume].type = Item_MintPerfume;
	//items[Item_MintPerfume].use = item_use_mintPerfume;
	gfc_word_cpy(items[Item_MintPerfume].name, "Mint Perfume");
	gfc_line_cpy(items[Item_MintPerfume].description, "Fiends hate the smell of mint!");
	gfc_line_cpy(items[Item_MintPerfume].obtainLine, "You find a bottle of Mint Perfume.");
	gfc_line_cpy(items[Item_MintPerfume].useUpLine, "The perfume bottle has been emptied.");

	items[Item_LemonPerfume].buyCost = 100;
	items[Item_LemonPerfume].consumeChance = 0.4;
	items[Item_LemonPerfume].targetingType = ITT_None;
	items[Item_LemonPerfume].useTime = 2;
	items[Item_LemonPerfume].type = Item_LemonPerfume;
	//items[Item_LemonPerfume].use = item_use_lemonPerfume;
	gfc_word_cpy(items[Item_LemonPerfume].name, "Lemon Perfume");
	gfc_line_cpy(items[Item_LemonPerfume].description, "Fiends love the smell of lemon!");
	gfc_line_cpy(items[Item_LemonPerfume].obtainLine, "You find a bottle of Lemon Perfume.");
	gfc_line_cpy(items[Item_LemonPerfume].useUpLine, "The perfume bottle has been emptied.");

	items[Item_SacredDagger].buyCost = 4500;
	items[Item_SacredDagger].consumeChance = 0.15;
	items[Item_SacredDagger].targetingType = ITT_Enemy;
	items[Item_SacredDagger].useTime = 1;
	items[Item_SacredDagger].type = Item_SacredDagger;
	//items[Item_SacredDagger].use = item_use_sacredDagger;
	gfc_word_cpy(items[Item_SacredDagger].name, "Sacred Dagger");
	gfc_line_cpy(items[Item_SacredDagger].description, "Can cause harm to fiends.");
	gfc_line_cpy(items[Item_SacredDagger].obtainLine, "You find a Sacred Dagger!");
	gfc_line_cpy(items[Item_SacredDagger].useUpLine, "The Sacred Dagger broke!");

	items[Item_SacredSword].buyCost = 0;
	items[Item_SacredSword].consumeChance = 0.15;
	items[Item_SacredSword].targetingType = ITT_Enemy;
	items[Item_SacredSword].useTime = 1;
	items[Item_SacredSword].type = Item_SacredSword;
	//items[Item_SacredSword].use = item_use_firstAid;
	gfc_word_cpy(items[Item_SacredSword].name, "Sacred Sword");
	gfc_line_cpy(items[Item_SacredSword].description, "Can cause substantial harm to fiends.");
	gfc_line_cpy(items[Item_SacredSword].obtainLine, "What luck! You find a Sacred Sword!!");
	gfc_line_cpy(items[Item_SacredSword].useUpLine, "The Sacred Sword shattered!");
}

void inventory_load() {
	SJson* save = sj_load("save/savegame.dat");
	if (!save) {
		slog("Could not load savegame.dat, aborting inventory load.");
		return;
	}

	if (sj_object_get_value(save, "funds")) {
		sj_get_integer_value(sj_object_get_value(save, "funds"), &inventory.funds);
	}
	else {
		slog("Player has no wallet yet.");
	}

	SJson* inv = sj_object_get_value(save, "items");
	if (!inv) {
		slog("Could not find item list, aborting inventory load.");
		sj_free(save);
		return;
	}
	int count = sj_array_get_count(inv);
	for (int i = 0; i < count; i++) {
		int itemType = 0;
		sj_get_integer_value(sj_array_get_nth(inv, i), &itemType);
		inventory_add_item(itemType);
	}
	sj_free(save);
}

void inventory_add_item(ItemType type) {
	gfc_list_append(inventory.items, &items[type]);
}
Item* inventory_get_item(int index) {
	return gfc_list_get_nth(inventory.items, index);
}
void inventory_remove_item(int index) {
	gfc_list_delete_nth(inventory.items, index);
}
void inventory_remove_item_by_ref(Item* item) {
	gfc_list_delete_data(inventory.items, item);
}
List* inventory_get_items() {
	return inventory.items;
}
int inventory_get_funds() {
	return inventory.funds;
}
void inventory_modify_funds(int change) {
	inventory.funds += change;
	if (inventory.funds < 0) inventory.funds = 0;
}


static int effectValue;
static int effectValue2;
static int extra = -1;

int item_use_firstAid(Item* self, FiendData* target, int* step) {
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s uses a First-Aid Kit.", ((PlayerData*)(player_get()->data))->name);
		if (in_battle()) {
			battle_set_main_dialogue(sbuf);
			battle_wait(750);
		}
		break;
	}
	case 1:
	{
		effectValue = max(target->stats[MHP] / 10, 1);
		target->HP = min(target->HP + effectValue, target->stats[MHP]);
		TextBlock sbuf;
		sprintf(sbuf, "%s uses a First-Aid Kit.\n%s recovers %i HP!", ((PlayerData*)(player_get()->data))->name,target->name,effectValue);
		slog(sbuf);
		if (in_battle()) {
			battle_set_main_dialogue(sbuf);
			battle_wait(1000);
		}
		break;
	}
	case 2:
		*step = 0;
		return -1;
	}
	*step += 1;
	return 0;
}
int item_use_magicDew(Item* self, FiendData* target, int* step) {
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s uses a phial of Magic Dew.", ((PlayerData*)(player_get()->data))->name);
		if (in_battle()) {
			battle_set_main_dialogue(sbuf);
			battle_wait(750);
		}
		break;
	}
	case 1:
	{
		effectValue = max(target->stats[MMP] / 10, 1);
		target->MP = min(target->MP + effectValue, target->stats[MMP]);
		TextBlock sbuf;
		sprintf(sbuf, "%s uses a phial of Magic Dew.\n%s recovers %i MP!", ((PlayerData*)(player_get()->data))->name, target->name, effectValue);
		slog(sbuf);
		if (in_battle()) {
			battle_set_main_dialogue(sbuf);
			battle_wait(1000);
		}
		break;
	}
	case 2:
		*step = 0;
		return -1;
	}
	*step += 1;
	return 0;
}
int item_use_soulstim(Item* self, FiendData* target, int* step) {
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s pours out a flask of Soulstim.", ((PlayerData*)(player_get()->data))->name);
		if (in_battle()) {
			battle_set_main_dialogue(sbuf);
			battle_wait(750);
		}
		break;
	}
	case 1:
	{
		effectValue = max(target->stats[MHP] * 0.3, 1);
		target->HP = min(target->HP + effectValue, target->stats[MHP]);
		target->isDead = false;
		TextBlock sbuf;
		sprintf(sbuf, "%s pours out a flask of Soulstim.\n%s has returned from the dead!", ((PlayerData*)(player_get()->data))->name, target->name, effectValue);
		slog(sbuf);
		if (in_battle()) {
			battle_set_main_dialogue(sbuf);
			battle_wait(1000);
		}
		break;
	}
	case 2:
		*step = 0;
		return -1;
	}
	*step += 1;
	return 0;
}
int item_use_spiritSpirits(Item* self, FiendData* target, int* step);
int item_use_bitterMedicine(Item* self, FiendData* target, int* step);
int item_use_portalSigil(Item* self, FiendData* target, int* step);
int item_use_mintPerfume(Item* self, FiendData* target, int* step);
int item_use_lemonPerfume(Item* self, FiendData* target, int* step);
int item_use_sacredDagger(Item* self, FiendData* target, int* step);
int item_use_sacredSword(Item* self, FiendData* target, int* step);