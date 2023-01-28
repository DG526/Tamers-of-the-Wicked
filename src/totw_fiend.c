#include "totw_fiend.h"
#include "gfc_input.h"

void dummy_think(Entity* self) {
	if (gfc_input_controller_button_pressed(0, "D_U"))
		self->position.y -= 1;
	else if (gfc_input_controller_button_held(0, "D_U"))
		self->position.y -= 0.1;
	if (gfc_input_controller_button_pressed(0, "D_D"))
		self->position.y += 1;
	else if (gfc_input_controller_button_held(0, "D_D"))
		self->position.y += 0.1;
	if (gfc_input_controller_button_pressed(0, "D_L"))
		self->position.x -= 1;
	else if (gfc_input_controller_button_held(0, "D_L"))
		self->position.x -= 0.1;
	if (gfc_input_controller_button_pressed(0, "D_R"))
		self->position.x += 1;
	else if (gfc_input_controller_button_held(0, "D_R"))
		self->position.x += 0.1;
}