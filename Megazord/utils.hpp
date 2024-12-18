#pragma once


/*
	Ensemble de fonction utiles pour l'IA MEGAZORD
*/

typedef void(*void_function_void)();

float map(float smin, float smax, float val, float omin, float omax);


// On doit toujours avoir 1000 Halite
#define MIN_HALITE_THRESHOLD 1000

#define STORAGE_DISTANCE_CLOSE 5
#define STORAGE_DISTANCE_FAR   10

// GAME CONSTANTS
#define MAX_TURN_NUMBER 500
#define COST_SPAWN_BOAT 1000
#define COST_CREATE_DROPOFF 4000
#define MAX_HALITE_IN_SHIP 1000
