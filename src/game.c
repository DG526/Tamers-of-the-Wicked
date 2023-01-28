#include <SDL.h>
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gfc_input.h"
#include "simple_logger.h"
#include "totw_entity.h"
#include "totw_fiend.h"

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    //Sprite *sprite;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    Color mouseColor = gfc_color8(255,100,255,200);
    Vector2D renderSize = vector2d(300, 180);
    Vector2D renderScale = vector2d(4, 4);
    
    /*program initializtion*/
    init_logger("gf2d.log");
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "Tamers of the Wicked",
        renderSize.x * renderScale.x,
        renderSize.y * renderScale.y,
        renderSize.x,
        renderSize.y,
        vector4d(200,200,200,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    SDL_ShowCursor(SDL_DISABLE);

    entity_manager_init(256);
    gfc_input_init("config/input_dualshock.cfg");
    
    /*demo setup*/
    //sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

    Sprite* monsterSprite1 = gf2d_sprite_load_all("images/enemies/Boarcupine_Battle.png", 64, 64, 1, 0);
    Sprite* monsterSprite2 = gf2d_sprite_load_all("images/enemies/Laugher_Battle.png", 64, 64, 1, 0);
    Sprite* monsterSprite3 = gf2d_sprite_load_all("images/enemies/Mothsquito_Battle.png", 64, 64, 1, 0);
    int spacing = 4;
    Entity* boarcupine = entity_new();
    if (boarcupine) {
        boarcupine->position = vector2d(renderSize.x / 2 - monsterSprite1->frame_w * 1.5 - spacing, renderSize.y / 2 - monsterSprite1->frame_h - spacing);
        boarcupine->sprite = monsterSprite1;
        slog("Boarcupine created @ (%f, %f).", boarcupine->position.x, boarcupine->position.y);
    }
    Entity* laugher = entity_new();
    if (laugher) {
        laugher->position = vector2d(renderSize.x / 2 - monsterSprite1->frame_w * 0.5, renderSize.y / 2 - monsterSprite1->frame_h - spacing);
        laugher->sprite = monsterSprite2;
        slog("Laugher created @ (%f, %f).", laugher->position.x, laugher->position.y);
    }
    Entity* mothsquito = entity_new();
    if (mothsquito) {
        mothsquito->position = vector2d(renderSize.x / 2 + monsterSprite1->frame_w * 0.5 + spacing, renderSize.y / 2 - monsterSprite1->frame_h - spacing);
        mothsquito->sprite = monsterSprite3;
        slog("Mothsquito created @ (%f, %f).", mothsquito->position.x, mothsquito->position.y);
        mothsquito->update = dummy_think;
    }
    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        gfc_input_update();

        entity_think_all();
        entity_update_all();

        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            //gf2d_sprite_draw_image(sprite,vector2d(0,0));
            
            //UI elements last
            entity_draw_all();
        Vector2D frac = vector2d(1 / renderScale.x, 1 / renderScale.y);
            /*gf2d_sprite_draw(
                mouse,
                vector2d(mx / renderScale.x,my / renderScale.y),
                &frac,
                NULL,
                NULL,
                NULL,
                &mouseColor,
                (int)mf);*/
        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
