// Bart Middag & Thomas Dhondt ; groepsnummer: 14 ; pathfinding.h

#ifndef H_PATH_FINDING
#define H_PATH_FINDING

#include "world.h"

int create_path(Path * path, World * world, Entity * from, Entity * to, TrajectoryType trajectory_type);

int refresh_path(Path * path, World * world);

void destroy_path(Path * path);

#endif