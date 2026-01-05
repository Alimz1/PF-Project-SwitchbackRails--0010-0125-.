#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

// ============================================================================
// SIMULATION_STATE.H - Global constants and state
// ============================================================================
// All gameplay data lives in simple globals and arrays.
// ============================================================================

// ----------------------------------------------------------------------------
// GRID CONSTANTS
// ----------------------------------------------------------------------------
#define MAX_ROWS 64
#define MAX_COLS 128

// ----------------------------------------------------------------------------
// TRAIN CONSTANTS
// ----------------------------------------------------------------------------
#define MAX_TRAINS 64
#define MAX_SPAWNS 32
#define MAX_DESTS 32

// ----------------------------------------------------------------------------
// SWITCH CONSTANTS
// ----------------------------------------------------------------------------
#define MAX_SWITCHES 26

// ----------------------------------------------------------------------------
// WEATHER CONSTANTS
// ----------------------------------------------------------------------------
#define WEATHER_NORMAL 0
#define WEATHER_RAIN 1
#define WEATHER_FOG 2

// ----------------------------------------------------------------------------
// SIGNAL CONSTANTS
// ----------------------------------------------------------------------------
#define SIGNAL_GREEN 0
#define SIGNAL_YELLOW 1
#define SIGNAL_RED 2

// ----------------------------------------------------------------------------
// DIRECTION CONSTANTS
// ----------------------------------------------------------------------------
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

// ----------------------------------------------------------------------------
// GLOBAL STATE: GRID
// ----------------------------------------------------------------------------
extern int g_rows;
extern int g_cols;
extern char g_grid[MAX_ROWS][MAX_COLS];
extern char g_levelName[128];
extern char g_levelPath[256];

// ----------------------------------------------------------------------------
// GLOBAL STATE: TRAINS
// ----------------------------------------------------------------------------
extern int g_totalScheduledTrains;
extern int g_trainSpawnTick[MAX_TRAINS];
extern int g_trainSpawnRow[MAX_TRAINS];
extern int g_trainSpawnCol[MAX_TRAINS];
extern int g_trainSpawnDir[MAX_TRAINS];
extern int g_trainDestIndex[MAX_TRAINS];
extern int g_trainRow[MAX_TRAINS];
extern int g_trainCol[MAX_TRAINS];
extern int g_trainDir[MAX_TRAINS];
extern int g_trainNextRow[MAX_TRAINS];
extern int g_trainNextCol[MAX_TRAINS];
extern int g_trainNextDir[MAX_TRAINS];
extern int g_activeTrainCount;
extern int g_arrivedTrainCount;
extern int g_crashedTrainCount;
extern bool g_trainSpawned[MAX_TRAINS];
extern bool g_trainActive[MAX_TRAINS];
extern bool g_trainArrived[MAX_TRAINS];
extern bool g_trainCrashed[MAX_TRAINS];
extern bool g_trainHasNext[MAX_TRAINS];

// ----------------------------------------------------------------------------
// GLOBAL STATE: SWITCHES (A-Z mapped to 0-25)
// ----------------------------------------------------------------------------
extern bool g_switchExists[MAX_SWITCHES];
extern int g_switchRow[MAX_SWITCHES];
extern int g_switchCol[MAX_SWITCHES];
extern int g_switchState[MAX_SWITCHES];
extern int g_switchCounter[MAX_SWITCHES];
extern bool g_switchFlipQueued[MAX_SWITCHES];
extern int g_signalColor[MAX_SWITCHES];

// ----------------------------------------------------------------------------
// GLOBAL STATE: SPAWN POINTS
// ----------------------------------------------------------------------------
extern int g_spawnRows[MAX_SPAWNS];
extern int g_spawnCols[MAX_SPAWNS];
extern int g_spawnCount;

// ----------------------------------------------------------------------------
// GLOBAL STATE: DESTINATION POINTS
// ----------------------------------------------------------------------------
extern int g_destRows[MAX_DESTS];
extern int g_destCols[MAX_DESTS];
extern int g_destCount;

// ----------------------------------------------------------------------------
// GLOBAL STATE: SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
extern long g_tick;
extern int g_seed;
extern int g_weather;
extern bool g_exitRequested;

// ----------------------------------------------------------------------------
// GLOBAL STATE: METRICS
// ----------------------------------------------------------------------------
extern long g_totalMoves;

// ----------------------------------------------------------------------------
// GLOBAL STATE: EMERGENCY HALT
// ----------------------------------------------------------------------------
extern bool g_emergencyActive;
extern int g_emergencyRow;
extern int g_emergencyCol;
extern int g_emergencyTimer;

// ----------------------------------------------------------------------------
// QUERY HELPERS (used by parameter-less functions)
// ----------------------------------------------------------------------------
extern int g_queryRow;
extern int g_queryCol;
extern int g_currentTrain;

// ----------------------------------------------------------------------------
// INITIALIZATION FUNCTION
// ----------------------------------------------------------------------------
// Resets all state before loading a new level.
void initializeSimulationState();

#endif
