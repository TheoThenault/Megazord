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

hlt::Direction directionAvailable(hlt::Game*, std::shared_ptr<hlt::Player>, hlt::Position*);

std::string posToStr(hlt::Position);
std::string dirToStr(hlt::Direction);

#ifdef _DEBUG
# define LOG(X) hlt::log::log(X);
#else
# define LOG(X)
#endif // DEBUG

// On doit toujours avoir 1000 Halite
#define PLAYER_MIN_HALITE_THRESHOLD 1000

#define STORAGE_DISTANCE_CLOSE 10
#define STORAGE_DISTANCE_FAR  30

#define DIFF_NOMBRE_BATEAU_RADIUS 10
#define DIFF_NOMBRE_BATEAU_THREAT_THRESHOLD -1

#define SHIP_TRANSFORM_INTO_DROPOFF 800
#define SHIP_FULL 900

#define ENEMIE_SEARCH_RADIUS 5
#define SHIP_VIEW_DISTANCE 10
#define HALITE_IS_CLOSE 5

#define MINE_HALITE_THRESHOLD 250
#define KEEP_HARVESTING_THRESHOLD 50

// GAME CONSTANTS
#define MAX_TURN_NUMBER 500
#define COST_SPAWN_BOAT 1000
#define COST_CREATE_DROPOFF 4000
#define MAX_HALITE_IN_SHIP 1000
#define MAX_NATURAL_HALITE 1000
