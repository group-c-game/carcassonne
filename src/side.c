#include <stdlib.h>
#include "side.h"

struct Side {
	element type;
	int completion;	// 0 - not set, -1 - not completed, 1 - completed
};

Side* Side_new(element type) {
	Side* self = (Side*)malloc(sizeof(Side));
	self->type = type;
	self->completion = 0;
	return self;
}

Side* Side_newDefault(void) {
	Side* self = (Side*)malloc(sizeof(Side));
	self->type = -1;
	self->completion = 0;
	return self;
}

void Side_free(Side** selfPtr) {
	Side* self = *selfPtr;
	free(self);
	*selfPtr = NULL;
}

element Side_getType(Side* self) {
	return self->type;
}

void Side_setType(Side* self, element type) {
	self->type = type;
}

int Side_isCompleted(Side* self) {
	return self->completion;
}

void Side_setCompletionStatus(Side* self, int compl) {
	self->completion = compl;
}

Direction Direction_getOpposite(Direction dir) {
	switch (dir) {
	case NORTH: return SOUTH;
	case EAST: return WEST;
	case SOUTH: return NORTH;
	case WEST: return EAST;
	}
	return -1;
}

bool Direction_areOpposite(Direction dir1, Direction dir2) {
	if ((dir1 == NORTH && dir2 == SOUTH) || (dir1 == SOUTH && dir2 == NORTH) || (dir1 == WEST && dir2 == EAST) || (dir1 == EAST && dir2 == WEST)) {
		return true;
	}
	return false;
}