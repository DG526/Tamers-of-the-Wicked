#include "simple_logger.h"
#include "totw_skill.h";
#include "totw_battle.h";

static Sound* battleSounds[7];
void battle_sounds_init() {
	battleSounds[SkS_Ready_Attack] = gfc_sound_load("audio/sfx/Attack_Normal.mp3", 1, 3);
	battleSounds[SkS_Ready_Attack_Special] = gfc_sound_load("audio/sfx/Attack_Special.mp3", 1, 3);
	battleSounds[SkS_Ready_Magic_Pos] = gfc_sound_load("audio/sfx/Magic_Pos.mp3", 1, 3);
	battleSounds[SkS_Ready_Magic_Neg] = gfc_sound_load("audio/sfx/Magic_Neg.mp3", 1, 3);
	battleSounds[SkS_Hit] = gfc_sound_load("audio/sfx/Hit.mp3", 1, 3);
	battleSounds[SkS_Kill] = gfc_sound_load("audio/sfx/Death.mp3", 1, 3);
	battleSounds[SkS_BossKill] = gfc_sound_load("audio/sfx/Death_Boss.mp3", 1, 3);
}
void battle_sounds_free() {
	for (int i = 0; i < 7; i++) {
		gfc_sound_free(battleSounds[i]);
	}
}

static int effectValue;
static float effectValueF;
static int extra = -1;


Skill skill_copy(Skill skill) {
	Skill newSkill;
	memcpy(newSkill.name, skill.name, sizeof(TextLine));
	memcpy(newSkill.original, skill.original, sizeof(TextLine));
	memcpy(newSkill.progression, skill.progression, sizeof(TextLine));
	newSkill.perform = skill.perform;
	for (int i = 0; i < 6; i++) {
		newSkill.requiredStats[i] = skill.requiredStats[i];
	}
	newSkill.turnBoost = skill.turnBoost;
	newSkill.chosen = true;
	newSkill.flag = skill.flag;
	newSkill.inUse = true;
	return newSkill;
}

Skill get_skill(TextLine skillName) {
	Skill skill = { 0 };
	if (!skillName) {
		skill.turnBoost = INT_MIN;
		return skill;
	}
	if (!gfc_line_cmp(skillName,"Attack")) {
		slog("Got attack.");
		skill.perform = skill_attack;
		gfc_line_cpy(skill.description, "A basic attack.");
		skill.targetingType = STT_Enemy_1;
		skill.tacticFavor[Destruction] = 2;
		skill.tacticFavor[Chaos] = 2;
		skill.tacticFavor[Support] = 1;
	}
	else if (!gfc_line_cmp(skillName, "Defend")) {
		slog("Got defend.");
		skill.turnBoost = 100000;
		gfc_line_cpy(skill.description, "Reduces incoming damage.");
		skill.targetingType = STT_Self;
		skill.tacticFavor[Support] = 6;
		skill.perform = skill_defend;
	}
	else if (!gfc_line_cmp(skillName, "Toughie Recruit")) {
		slog("Got recruit (toughie).");
		skill.turnBoost = 10000;
		skill.perform = skill_recruit_toughie;
		skill.targetingType = STT_Enemy_1;
		skill.flag = SF_Recruiting;
	}
	else if (!gfc_line_cmp(skillName, "Trickster Recruit")) {
		slog("Got recruit (trickster).");
		skill.turnBoost = 10000;
		skill.perform = skill_recruit_trickster;
		skill.targetingType = STT_Enemy_1;
		skill.flag = SF_Recruiting;
	}
	else if (!gfc_line_cmp(skillName, "Mage Recruit")) {
		slog("Got recruit (mage).");
		skill.turnBoost = 10000;
		skill.perform = skill_recruit_mage;
		skill.targetingType = STT_Enemy_1;
		skill.flag = SF_Recruiting;
	}
	else if (!gfc_line_cmp(skillName, "Umbraboom")) {
		slog("Got Umbraboom.");
		skill.manaCost = 10;
		skill.turnBoost = -18;
		skill.perform = skill_umbraboom;
		memcpy(skill.description, "Summons an explosion of Darkness.", sizeof(TextLine));
		skill.targetingType = STT_Enemy_All;
		skill.tacticFavor[Destruction] = 12;
		skill.tacticFavor[Chaos] = 6;
		skill.requiredStats[MMP] = 45;
		skill.requiredStats[PWR] = 45;
		gfc_line_cpy(skill.progression, "Umbradokaan");
	}
	else if (!gfc_line_cmp(skillName, "Umbradokaan")) {
		slog("Got Umbradokaan.");
		skill.manaCost = 18;
		skill.turnBoost = -25;
		skill.perform = skill_umbradokaan;
		memcpy(skill.description, "Summons a mighty explosion \nof Darkness!", sizeof(TextLine));
		skill.targetingType = STT_Enemy_All;
		skill.tacticFavor[Destruction] = 17;
		skill.tacticFavor[Chaos] = 6;
		skill.requiredStats[MMP] = 100;
		skill.requiredStats[PWR] = 100;
	}
	else if (!gfc_line_cmp(skillName, "SparkBreath")) {
		slog("Got SparkBreath.");
		skill.manaCost = 10;
		skill.turnBoost = -9;
		skill.perform = skill_sparkbreath;
		memcpy(skill.description, "A fiery breath attack.", sizeof(TextLine));
		skill.targetingType = STT_Enemy_All;
		skill.tacticFavor[Destruction] = 9;
		skill.tacticFavor[Chaos] = 7;
		skill.requiredStats[MHP] = 50;
		skill.requiredStats[MMP] = 45;
		skill.requiredStats[PWR] = 30;
		skill.requiredStats[AGL] = 40;
		gfc_line_cpy(skill.progression, "FireGale");
	}
	else if (!gfc_line_cmp(skillName, "ChillBreath")) {
		slog("Got ChillBreath.");
		skill.manaCost = 10;
		skill.turnBoost = -9;
		skill.perform = skill_chillbreath;
		memcpy(skill.description, "An icy breath attack.", sizeof(TextLine));
		skill.targetingType = STT_Enemy_All;
		skill.tacticFavor[Destruction] = 9;
		skill.tacticFavor[Chaos] = 7;
		skill.requiredStats[MHP] = 50;
		skill.requiredStats[MMP] = 45;
		skill.requiredStats[PWR] = 30;
		skill.requiredStats[AGL] = 40;
		gfc_line_cpy(skill.progression, "FrostGale");
	}
	else if (!gfc_line_cmp(skillName, "FastHit")) {
		slog("Got FastHit.");
		skill.manaCost = 4;
		skill.turnBoost = 10000000;
		skill.perform = skill_fasthit;
		memcpy(skill.description, "A weak hit that always goes first.", sizeof(TextLine));
		skill.targetingType = STT_Enemy_1;
		skill.tacticFavor[Destruction] = 1;
		skill.tacticFavor[Chaos] = 2;
		skill.requiredStats[MMP] = 5;
		skill.requiredStats[ATK] = 25;
		skill.requiredStats[AGL] = 30;
	}
	else if (!gfc_line_cmp(skillName, "PowerHit")) {
		slog("Got PowerHit.");
		skill.manaCost = 7;
		skill.turnBoost = -10000000;
		skill.perform = skill_powerhit;
		memcpy(skill.description, "A strong hit that always goes last.", sizeof(TextLine));
		skill.targetingType = STT_Enemy_1;
		skill.tacticFavor[Destruction] = 8;
		skill.tacticFavor[Chaos] = 2;
		skill.requiredStats[ATK] = 60;
	}
	else {
		skill.turnBoost = INT_MIN;
		return skill;
	}
	memcpy(skill.name, skillName, sizeof(TextLine));
	skill.chosen = true;
	skill.inUse = true;
	return skill;
}

int skill_attack(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData = battle_get_party_member(targetParty, targetFiend);
	if(extra != -1)
		targetData = battle_get_party_member(targetParty, extra);
	else if (!targetData || targetData->HP == 0) { //If target is undeclared or already dead, pick a new random target.
		slog("No target selected, picking random target now.");
		while (extra == -1) {
			int n = ((int)(gfc_random() * 4)) % 4 + 1;
			//slog("Checking opponent %i-%i", targetParty, n);
			targetData = battle_get_party_member(targetParty, n);
			if (targetData && targetData->HP > 0) { 
				extra = n;
				slog("Got random target.");
			}
		}
		targetData = battle_get_party_member(targetParty, extra);
	}
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s attacks!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Attack], 0, 1, -1, -1);
		battle_wait(750);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.15;
		int raw = (userData->stats[ATK] * 4 - targetData->stats[DEF] * 3) / 8 * (1 + variation);
		effectValue = fiend_damage(targetData, raw, Elm_Neutral);
		TextBlock sbuf;
		sprintf(sbuf, "%s attacks!\nDeals %i damage to %s!", userData->name, effectValue, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if (effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData->HP > 0) {
			extra = -1;
			*step = 0;
			return 1;
		}
		break;
	}
	case 2:
	{
		if (targetData->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s attacks!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData->name, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData->isDead = true;
		if (targetData->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData->party == 2) battle_add_exp(targetData->exp);
		break;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}
int skill_defend(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	char* sbuf[GFCTEXTLEN];
	fiend_apply_status(userData, FS_Defending, 0);
	sprintf(sbuf, "%s defends.", userData->name);
	slog(sbuf);
	battle_set_main_dialogue(sbuf);
	battle_wait(1000);
	*step = 0;
	return 1;
}

int skill_recruit_toughie(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData = battle_get_party_member(targetParty, targetFiend);
	if (!targetData || targetData->HP == 0) { //If target is undeclared or already dead, pick a new random target.
		slog("No target selected, aborting.");
		return 1;
	}
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s flexes real hard!", userData->name);
		battle_set_main_dialogue(sbuf);
		battle_wait(500);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.05;
		float raw = (userData->stats[ATK] * 4 - targetData->stats[ATK] * 3) / 12.0f * (1 + variation) * 0.01f;
		effectValueF = fiend_impress(targetData, raw);
		TextLine impression;
		if (effectValueF >= 1) {
			sprintf(impression, "%s is thoroughly impressed!", targetData->name);
		}
		else if (effectValueF >= 0.6) {
			sprintf(impression, "%s is extremely impressed!", targetData->name);
		}
		else if (effectValueF >= 0.18) {
			sprintf(impression, "%s is impressed!", targetData->name);
		}
		else if (effectValueF > 0) {
			sprintf(impression, "%s is not very impressed.", targetData->name);
		}
		else {
			sprintf(impression, "%s is unimpressed.", targetData->name);
		}
		TextBlock sbuf;
		sprintf(sbuf, "%s flexes real hard!\n%s\nCurrent odds of recruitment: %i%%", userData->name, impression, (int)(battle_get_recruit_chance() * 100));
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		battle_wait(1050);
		break;
	}
	case 2:
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}

int skill_recruit_trickster(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData = battle_get_party_member(targetParty, targetFiend);
	if (!targetData || targetData->HP == 0) { //If target is undeclared or already dead, pick a new random target.
		slog("No target selected, aborting.");
		return 1;
	}
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s does an interesting dance!", userData->name);
		battle_set_main_dialogue(sbuf);
		battle_wait(500);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.05;
		float raw = (userData->stats[AGL] * 4 - targetData->stats[AGL] * 3) / 12.0f * (1 + variation) * 0.01f;
		effectValueF = fiend_impress(targetData, raw);
		TextLine impression;
		if (effectValueF >= 1) {
			sprintf(impression, "%s is thoroughly impressed!", targetData->name);
		}
		else if (effectValueF >= 0.6) {
			sprintf(impression, "%s is extremely impressed!", targetData->name);
		}
		else if (effectValueF >= 0.18) {
			sprintf(impression, "%s is impressed!", targetData->name);
		}
		else if (effectValueF > 0) {
			sprintf(impression, "%s is not very impressed.", targetData->name);
		}
		else {
			sprintf(impression, "%s is unimpressed.", targetData->name);
		}
		TextBlock sbuf;
		sprintf(sbuf, "%s does an interesting dance!\n%s\nCurrent odds of recruitment: %i%%", userData->name, impression, (int)(battle_get_recruit_chance() * 100));
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		battle_wait(1050);
		break;
	}
	case 2:
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}

int skill_recruit_mage(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData = battle_get_party_member(targetParty, targetFiend);
	if (!targetData || targetData->HP == 0) { //If target is undeclared or already dead, pick a new random target.
		slog("No target selected, aborting.");
		return 1;
	}
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s rattles off a bunch of trivia!", userData->name);
		battle_set_main_dialogue(sbuf);
		battle_wait(500);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.05;
		float raw = (userData->stats[PWR] * 4 - targetData->stats[PWR] * 3) / 12.0f * (1 + variation) * 0.01f;
		effectValueF = fiend_impress(targetData, raw);
		TextLine impression;
		if (effectValueF >= 1) {
			sprintf(impression, "%s is thoroughly impressed!", targetData->name);
		}
		else if (effectValueF >= 0.6) {
			sprintf(impression, "%s is extremely impressed!", targetData->name);
		}
		else if (effectValueF >= 0.18) {
			sprintf(impression, "%s is impressed!", targetData->name);
		}
		else if (effectValueF > 0) {
			sprintf(impression, "%s is not very impressed.", targetData->name);
		}
		else {
			sprintf(impression, "%s is unimpressed.", targetData->name);
		}
		TextBlock sbuf;
		sprintf(sbuf, "%s rattles off a bunch of trivia!\n%s\nCurrent odds of recruitment: %i%%", userData->name, impression, (int)(battle_get_recruit_chance() * 100));
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		battle_wait(1050);
		break;
	}
	case 2:
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}

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

int skill_umbraboom(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData[4] = { battle_get_party_member(targetParty, 1),battle_get_party_member(targetParty, 2),battle_get_party_member(targetParty, 3),battle_get_party_member(targetParty, 4) };
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s casts Umbraboom!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Magic_Neg], 0, 1, -1, -1);
		extra = 0;
		userData->MP -= 10;
		battle_wait(1000);
		break;
	}
	case 1:
	{
		if (extra > 3) {
			extra = -1;
			*step = 0;
			return 1;
		}
		if (!targetData[extra] || targetData[extra]->isDead) {
			extra++;
			return 0;
		}
		float variation = gfc_crandom() * 0.2;
		int raw = ((max(0, userData->stats[PWR] * 3.5 - targetData[extra]->stats[PWR] * 2 - targetData[extra]->stats[DEF]) / 6.5) + 30) * (1 + variation);
		effectValue = fiend_damage(targetData[extra], raw, Elm_Dark);
		TextBlock sbuf;
		sprintf(sbuf, "%s casts Umbraboom!\nDeals %i damage to %s!", userData->name, effectValue, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if (effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData[extra]->HP > 0) {
			extra++;
			*step = 1;
			if (extra > 3) {
				extra = -1;
				*step = 0;
				return 1;
			}
			else return 0;
		}
		break;
	}
	case 2:
	{
		if (targetData[extra]->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s casts Umbraboom!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData[extra]->name, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData[extra]->isDead = true;
		if (targetData[extra]->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData[extra]->party == 2) battle_add_exp(targetData[extra]->exp);
		extra++;
		*step = 1;
		return 0;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}
int skill_umbradokaan(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData[4] = { battle_get_party_member(targetParty, 1),battle_get_party_member(targetParty, 2),battle_get_party_member(targetParty, 3),battle_get_party_member(targetParty, 4) };
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s casts Umbradokaan!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Magic_Neg], 0, 1, -1, -1);
		extra = 0;
		userData->MP -= 10;
		battle_wait(1000);
		break;
	}
	case 1:
	{
		if (extra > 3) {
			extra = -1;
			*step = 0;
			return 1;
		}
		if (!targetData[extra] || targetData[extra]->isDead) {
			extra++;
			return 0;
		}
		float variation = gfc_crandom() * 0.2;
		int raw = ((max(0, userData->stats[PWR] * 5.5 - targetData[extra]->stats[PWR] * 2.2 - targetData[extra]->stats[DEF]) / 6.5) + 100) * (1 + variation);
		effectValue = fiend_damage(targetData[extra], raw, Elm_Dark);
		TextBlock sbuf;
		sprintf(sbuf, "%s casts Umbradokaan!\nDeals %i damage to %s!", userData->name, effectValue, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if (effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData[extra]->HP > 0) {
			extra++;
			*step = 1;
			if (extra > 3) {
				extra = -1;
				*step = 0;
				return 1;
			}
			else return 0;
		}
		break;
	}
	case 2:
	{
		if (targetData[extra]->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s casts Umbradokaan!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData[extra]->name, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData[extra]->isDead = true;
		if (targetData[extra]->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData[extra]->party == 2) battle_add_exp(targetData[extra]->exp);
		extra++;
		*step = 1;
		return 0;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}


int skill_sparkbreath(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData[4] = { battle_get_party_member(targetParty, 1),battle_get_party_member(targetParty, 2),battle_get_party_member(targetParty, 3),battle_get_party_member(targetParty, 4) };
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s lets loose a blistering wind!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Attack_Special], 0, 1, -1, -1);
		extra = 0;
		userData->MP -= 10;
		battle_wait(1000);
		break;
	}
	case 1:
	{
		if (extra > 3) {
			extra = -1;
			*step = 0;
			return 1;
		}
		if (!targetData[extra] || targetData[extra]->isDead) {
			extra++;
			return 0;
		}
		float variation = gfc_crandom() * 0.13;
		int raw = (max(0, min(userData->stats[MHP] * 0.6, 150) * userData->size - targetData[extra]->stats[DEF] * 1.2) / 4.5 + 20) * targetData[extra]->size * (1 + variation);
		effectValue = fiend_damage(targetData[extra], raw, Elm_Fire);
		TextBlock sbuf;
		sprintf(sbuf, "%s lets loose a blistering wind!\nDeals %i damage to %s!", userData->name, effectValue, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if (effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData[extra]->HP > 0) {
			extra++;
			*step = 1;
			if (extra > 3) {
				extra = -1;
				*step = 0;
				return 1;
			}
			else return 0;
		}
		break;
	}
	case 2:
	{
		if (targetData[extra]->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s lets loose a blistering wind!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData[extra]->name, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData[extra]->isDead = true;
		if (targetData[extra]->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData[extra]->party == 2) battle_add_exp(targetData[extra]->exp);
		extra++;
		*step = 1;
		return 0;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}
int skill_firegale(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_chillbreath(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData[4] = { battle_get_party_member(targetParty, 1),battle_get_party_member(targetParty, 2),battle_get_party_member(targetParty, 3),battle_get_party_member(targetParty, 4) };
	switch (*step) {
	case 0:
	{
		TextBlock sbuf;
		sprintf(sbuf, "%s lets loose an icy wind!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Attack_Special], 0, 1, -1, -1);
		extra = 0;
		userData->MP -= 10;
		battle_wait(1000);
		break;
	}
	case 1:
	{
		if (extra > 3) {
			extra = -1;
			*step = 0;
			return 1;
		}
		if (!targetData[extra] || targetData[extra]->isDead) {
			extra++;
			return 0;
		}
		float variation = gfc_crandom() * 0.1;
		int raw = (max(0, min(userData->stats[MHP] * 0.6, 150) * userData->size - targetData[extra]->stats[DEF] * 1.2) / 4.5 + 20) * targetData[extra]->size * (1 + variation);
		effectValue = fiend_damage(targetData[extra], raw, Elm_Ice);
		TextBlock sbuf;
		sprintf(sbuf, "%s lets loose an icy wind!\nDeals %i damage to %s!", userData->name, effectValue, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if(effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData[extra]->HP > 0) {
			extra++;
			*step = 1;
			if (extra > 3) {
				extra = -1;
				*step = 0;
				return 1;
			}
			else return 0;
		}
		break;
	}
	case 2:
	{
		if (targetData[extra]->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s lets loose an icy wind!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData[extra]->name, targetData[extra]->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData[extra]->isDead = true;
		if (targetData[extra]->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData[extra]->party == 2) battle_add_exp(targetData[extra]->exp);
		extra++;
		*step = 1;
		return 0;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}
int skill_frostgale(int userParty, int userFiend, int targetParty, int targetFiend, int* step);

int skill_fasthit(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData = battle_get_party_member(targetParty, targetFiend);
	if (extra != -1)
		targetData = battle_get_party_member(targetParty, extra);
	else if (!targetData || targetData->HP == 0) { //If target is undeclared or already dead, pick a new random target.
		slog("No target selected, picking random target now.");
		while (extra == -1) {
			int n = ((int)(gfc_random() * 4)) % 4 + 1;
			//slog("Checking opponent %i-%i", targetParty, n);
			targetData = battle_get_party_member(targetParty, n);
			if (targetData && targetData->HP > 0) {
				extra = n;
				slog("Got random target.");
			}
		}
		targetData = battle_get_party_member(targetParty, extra);
	}
	switch (*step) {
	case 0:
	{
		userData->MP -= 3;
		TextBlock sbuf;
		sprintf(sbuf, "%s strikes with extreme speed!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Attack], 0, 1, -1, -1);
		battle_wait(750);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.1;
		int raw = (userData->stats[ATK] * 4 - targetData->stats[DEF] * 3) / 5 * (1 + variation) * 0.4;
		effectValue = fiend_damage(targetData, raw, Elm_Neutral);
		TextBlock sbuf;
		sprintf(sbuf, "%s strikes with extreme speed!\nDeals %i damage to %s!", userData->name, effectValue, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if (effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData->HP > 0) {
			extra = -1;
			*step = 0;
			return 1;
		}
		break;
	}
	case 2:
	{
		if (targetData->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s strikes with extreme speed!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData->name, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData->isDead = true;
		if (targetData->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData->party == 2) battle_add_exp(targetData->exp);
		break;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}
int skill_powerhit(int userParty, int userFiend, int targetParty, int targetFiend, int* step) {
	slog("%i, %i on %i, %i", userParty, userFiend, targetParty, targetFiend);
	FiendData* userData = battle_get_party_member(userParty, userFiend);
	FiendData* targetData = battle_get_party_member(targetParty, targetFiend);
	if (extra != -1)
		targetData = battle_get_party_member(targetParty, extra);
	else if (!targetData || targetData->HP == 0) { //If target is undeclared or already dead, pick a new random target.
		slog("No target selected, picking random target now.");
		while (extra == -1) {
			int n = ((int)(gfc_random() * 4)) % 4 + 1;
			//slog("Checking opponent %i-%i", targetParty, n);
			targetData = battle_get_party_member(targetParty, n);
			if (targetData && targetData->HP > 0) {
				extra = n;
				slog("Got random target.");
			}
		}
		targetData = battle_get_party_member(targetParty, extra);
	}
	switch (*step) {
	case 0:
	{
		userData->MP -= 7;
		TextBlock sbuf;
		sprintf(sbuf, "%s delivers a powerful blow!", userData->name);
		battle_set_main_dialogue(sbuf);
		gfc_sound_play(battleSounds[SkS_Ready_Attack], 0, 1, -1, -1);
		battle_wait(750);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.15;
		int raw = (userData->stats[ATK] * 4 - targetData->stats[DEF] * 3) / 5 * (1 + variation) * 1.5;
		effectValue = fiend_damage(targetData, raw, Elm_Neutral);
		TextBlock sbuf;
		sprintf(sbuf, "%s delivers a powerful blow!\nDeals %i damage to %s!", userData->name, effectValue, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		if (effectValue > 0)
			gfc_sound_play(battleSounds[SkS_Hit], 0, 1, -1, -1);
		battle_wait(1000);
		if (targetData->HP > 0) {
			extra = -1;
			*step = 0;
			return 1;
		}
		break;
	}
	case 2:
	{
		if (targetData->HP > 0) break;
		TextBlock sbuf;
		sprintf(sbuf, "%s delivers a powerful blow!\nDeals %i damage to %s!\n%s has fallen.", userData->name, effectValue, targetData->name, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		targetData->isDead = true;
		if (targetData->hasBossSound)
		{
			gfc_sound_play(battleSounds[SkS_BossKill], 0, 1, -1, -1);
			battle_wait(2600);
		}
		else {
			gfc_sound_play(battleSounds[SkS_Kill], 0, 1, -1, -1);
			battle_wait(1000);
		}
		if (targetData->party == 2) battle_add_exp(targetData->exp);
		break;
	}
	case 3:
		extra = -1;
		*step = 0;
		return 1;
	}
	*step += 1;
	return 0;
}