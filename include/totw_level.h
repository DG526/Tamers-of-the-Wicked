#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gfc_text.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "totw_entity.h"

typedef struct
{
    int tileFrame;
    Vector2D coords;
    int solid;  // if true the tile cannot be walked through
    Entity* occupier;
    int encounterZone; //Determines what random battles can be fought on this tile. If 0, no random battles.
}TileInfo;

typedef struct {
    int tiles;
    Vector2D tileSize;
    int fpl;
    Sprite* sprite;
    TileInfo templates[64];
}Tileset;

typedef struct
{
    TextLine    name;
    Tileset tileSet;
    Vector2D    mapSize;    //the width and height of the tile map
    int* tileMap;    //WTF???
    Sprite* tileLayer;  //TBD
    //List* clips;      //list of static shapes to clip against in the game world
    TileInfo* tileInfoMap;
    int encounterSteps;
    int encounterVariance;
}Level;

/**
 * @brief get the set active level pointer if it is set
 * @return NULL if no active level, or a pointer to it
 */
Level* level_get_active_level();

/**
 * @brief check if a shape clips any of the level static shapes
 * @param level the level to check
 * @param shape the shape to check with
 * @return 0 if there is no overlap, 1 if there is
 */
//int level_shape_clip(Level* level, Shape shape);

/**
 * @brief set the active level (or turn it off)
 * @param level the level to set it to (or NULL to turn off the level)
 */
void level_set_active_level(Level* level);

/**
 * @brief load a level from a json file
 * @param filename path to the json file containing level data
 * @return NULL on not found or other error, or the loaded level
 */
Level* level_load(const char* filename);

/**
 * @brief draw a level
 * @param level the level to draw
 */
void level_draw(Level* level);

/**
 * @brief allocate a blank level struct
 * @return NULL on error or an intialized level
 */
Level* level_new();

/**
 * @brief clean up a previously allocated level
 */
void level_free(Level* level);

TileInfo tile_get(int column, int row);
void tile_cpy(TileInfo* dst, TileInfo* src);
void tile_set_occupier(Level* level, int column, int row, Entity* occupier);
int tile_is_available(Level* level, int column, int row);
void tile_set_availability(Level* level, int column, int row, Bool available);
Entity* tile_get_occupier(Level* level, int column, int row);

Tileset tileset_load(const char* filename);


#endif