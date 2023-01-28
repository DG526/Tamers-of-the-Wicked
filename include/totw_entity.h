#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gfc_vector.h"

#include "gf2d_sprite.h"

typedef struct Entity_S
{
	Bool inuse;
	Sprite* sprite;
	float frame;
	Vector2D position;
	Vector2D scale;

	void (*update)(struct Entity_S* self);
	void (*think)(struct Entity_S* self);
}Entity;

/**
* @brief Initializes manager for entity system.
* @note Automatically queues up close function for program exit.
* @param max: Maximum number of supported entities at a given time.
*/
void entity_manager_init(Uint32 max);

/**
* @brief 
*/
Entity* entity_new();

void entity_free(Entity* ent);
void entity_free_all();

void entity_draw(Entity* ent);
void entity_draw_all();

void entity_update(Entity* ent);
void entity_update_all();

void entity_think(Entity* ent);
void entity_think_all();

#endif