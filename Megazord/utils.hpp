#pragma once

#include "../hlt/game.hpp"
#include "../hlt/player.hpp"

/*
	Ensemble de fonction utiles pour l'IA MEGAZORD
*/

typedef void(*void_function_void)();

float map(float smin, float smax, float val, float omin, float omax);

int random(int min, int max);

int diffNombreBateau(hlt::Game*, std::shared_ptr<hlt::Player>, hlt::Position*);
int nombreEnemies(hlt::Game*, std::shared_ptr<hlt::Player>, hlt::Position*);


// On doit toujours avoir 1000 Halite
#define PLAYER_MIN_HALITE_THRESHOLD 1000

#define STORAGE_DISTANCE_CLOSE 5
#define STORAGE_DISTANCE_FAR   10

#define DIFF_NOMBRE_BATEAU_RADIUS 10

#define DIFF_NOMBRE_BATEAU_THREAT_THRESHOLD -1

#define ENEMIE_SEARCH_RADIUS 10

#define MINE_HALITE_THRESHOLD 250

// GAME CONSTANTS
#define MAX_TURN_NUMBER 500
#define COST_SPAWN_BOAT 1000
#define COST_CREATE_DROPOFF 4000
#define MAX_HALITE_IN_SHIP 1000
