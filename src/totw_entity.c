#include "simple_logger.h"
#include "totw_camera.h"
#include "totw_entity.h"
#include "totw_fiend.h"
#include "totw_game_status.h"


typedef struct {
	Uint32 entityMax;
	Entity* entityList;
}EntityManager;

static EntityManager entityManager = { 0 };

void entity_manager_close() {
	if (entityManager.entityList)
		free(entityManager.entityList);
	memset(&entityManager, 0, sizeof(EntityManager));
	slog("Entity system closed.");
}

void entity_manager_init(Uint32 max) {
	if (max <= 0) {
		slog("Cannot initialize entity system: < 1 enitities specified!");
		return;
	}
	entityManager.entityList = gfc_allocate_array(sizeof(Entity), max);
	if (!entityManager.entityList) {
		slog("Failed to initialize entity system!");
		return;
	}
	entityManager.entityMax = max;
	atexit(entity_manager_close);
	slog("Entity system initialized.");
}

Entity* entity_new() {
	for (int i = 0; i < entityManager.entityMax; i++) {
		if (entityManager.entityList[i].inuse) continue;
		entityManager.entityList[i].inuse = 1;
		entityManager.entityList[i].position = vector2d(0, 0);
		entityManager.entityList[i].scale = vector2d(1, 1);
		entityManager.entityList[i].color = gfc_color(1,1,1,1);
		entityManager.entityList[i].frame = 0;
		return &entityManager.entityList[i];
	}
	return NULL;
}

void entity_free(Entity* ent) {
	if (!ent) {
		slog("No entity provided for freeing.");
		return;
	}
	if (ent->type == ET_Fiend) {
		FiendData* data = (FiendData*)(ent->data);
		gf2d_sprite_free(data->sprite);
		//memset(data, 0, sizeof(FiendData));
	}
	if(ent->data)
		free(ent->data);
	if (ent->sprite)
		gf2d_sprite_free(ent->sprite);
	memset(ent, 0, sizeof(Entity));
}
void entity_free_leave_data(Entity* ent) {
	if (!ent) {
		slog("No entity provided for freeing.");
		return;
	}if (ent->sprite)
		gf2d_sprite_free(ent->sprite);
	memset(ent, 0, sizeof(Entity));
}

void entity_free_all() {

	for (int i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i].inuse) continue;
		entity_free(&(entityManager.entityList[i]));
	}
}
void entity_free_interactibles() {
	for (int i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i].inuse || entityManager.entityList[i].type != ET_Interactible) continue;
		entity_free(&(entityManager.entityList[i]));
	}
}

void entity_draw(Entity* ent) {
	if (!ent) return;
	if (ent->type == ET_Fiend) {
		FiendData* data = (FiendData*)(ent->data);
		if (data->isDead) return;
	}
	if ((ent->type == ET_Player || ent->type == ET_Interactible) && (game_get_state() == GS_Battle || game_get_state() == GS_Naming || game_get_state() == GS_Title))
		return;
	Vector2D drawPos;
	vector2d_add(drawPos, ent->position, camera_get_draw_offset());
	vector2d_add(drawPos, drawPos, ent->drawOffset);
	if (ent->sprite) {
		gf2d_sprite_draw(
			ent->sprite,
			drawPos,
			&(ent->scale),
			NULL,
			NULL,
			NULL,
			&(ent->color),
			ent->frame
		);
	}
}

void entity_draw_all() {
	for (int l = 15; l > -15; l--) {
		for (int i = 0; i < entityManager.entityMax; i++) {
			if (!entityManager.entityList[i].inuse || entityManager.entityList[i].drawDepth != l) continue;
			entity_draw(&(entityManager.entityList[i]));
		}
	}
}

void entity_update(Entity* ent) {
	if (!ent) return;
	ent->frame += ent->frameSpeed;
	if (ent->frame >= ent->frameMax + 1)
		ent->frame = ent->frameMin;
	if (ent->update) ent->update(ent);
}

void entity_update_all() {

	for (int i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i].inuse) continue;
		entity_update(&(entityManager.entityList[i]));
	}
}

void entity_think(Entity* ent) {
	if (!ent) return;
	if (ent->think) ent->think(ent);
}

void entity_think_all() {

	for (int i = 0; i < entityManager.entityMax; i++) {
		if (!entityManager.entityList[i].inuse) continue;
		entity_think(&(entityManager.entityList[i]));
	}
}