#include "simple_logger.h"
#include "totw_skill.h";
#include "totw_battle.h";

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
	return newSkill;
}

Skill get_skill(TextLine skillName) {
	Skill skill = { 0 };
	if (!skillName) return skill;
	if (!gfc_line_cmp(skillName,"Attack")) {
		slog("Got attack.");
		skill.perform = skill_attack;
	}
	else if (!gfc_line_cmp(skillName, "Defend")) {
		slog("Got defend.");
		skill.turnBoost = 100000;
		skill.perform = skill_defend;
	}
	else if (!gfc_line_cmp(skillName, "Toughie Recruit")) {
		slog("Got recruit (toughie).");
		skill.turnBoost = 10000;
		skill.perform = skill_recruit_toughie;
		skill.flag = SF_Recruiting;
	}
	else if (!gfc_line_cmp(skillName, "Trickster Recruit")) {
		slog("Got recruit (trickster).");
		skill.turnBoost = 10000;
		skill.perform = skill_recruit_trickster;
		skill.flag = SF_Recruiting;
	}
	else if (!gfc_line_cmp(skillName, "Mage Recruit")) {
		slog("Got recruit (mage).");
		skill.turnBoost = 10000;
		skill.perform = skill_recruit_mage;
		skill.flag = SF_Recruiting;
	}
	else return;
	memcpy(skill.name, skillName, sizeof(TextLine));
	skill.chosen = true;
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
		battle_wait(500);
		break;
	}
	case 1:
	{
		float variation = gfc_crandom() * 0.15;
		int raw = (userData->stats[ATK] * 4 - targetData->stats[DEF] * 3) / 5 * (1 + variation);
		effectValue = fiend_damage(targetData, raw);
		TextBlock sbuf;
		sprintf(sbuf, "%s attacks!\nDeals %i damage to %s!", userData->name, effectValue, targetData->name);
		slog(sbuf);
		battle_set_main_dialogue(sbuf);
		battle_wait(700);
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
		battle_wait(700);
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
	battle_wait(700);
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
