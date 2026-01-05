#include "simulation_state.h"

using namespace std;

// ============================================================================
// SIMULATION_STATE.CPP - Global state definitions
// ============================================================================

// ----------------------------------------------------------------------------
// GRID
// ----------------------------------------------------------------------------
int g_rows = 0;
int g_cols = 0;
char g_grid[MAX_ROWS][MAX_COLS];
char g_levelName[128];
char g_levelPath[256];

// ----------------------------------------------------------------------------
// TRAINS
// ----------------------------------------------------------------------------
int g_totalScheduledTrains = 0;
int g_trainSpawnTick[MAX_TRAINS];
int g_trainSpawnRow[MAX_TRAINS];
int g_trainSpawnCol[MAX_TRAINS];
int g_trainSpawnDir[MAX_TRAINS];
int g_trainDestIndex[MAX_TRAINS];
int g_trainRow[MAX_TRAINS];
int g_trainCol[MAX_TRAINS];
int g_trainDir[MAX_TRAINS];
int g_trainNextRow[MAX_TRAINS];
int g_trainNextCol[MAX_TRAINS];
int g_trainNextDir[MAX_TRAINS];
int g_activeTrainCount = 0;
int g_arrivedTrainCount = 0;
int g_crashedTrainCount = 0;
bool g_trainSpawned[MAX_TRAINS];
bool g_trainActive[MAX_TRAINS];
bool g_trainArrived[MAX_TRAINS];
bool g_trainCrashed[MAX_TRAINS];
bool g_trainHasNext[MAX_TRAINS];

// ----------------------------------------------------------------------------
// SWITCHES
// ----------------------------------------------------------------------------
bool g_switchExists[MAX_SWITCHES];
int g_switchRow[MAX_SWITCHES];
int g_switchCol[MAX_SWITCHES];
int g_switchState[MAX_SWITCHES];
int g_switchCounter[MAX_SWITCHES];
bool g_switchFlipQueued[MAX_SWITCHES];
int g_signalColor[MAX_SWITCHES];

// ----------------------------------------------------------------------------
// SPAWN AND DESTINATION POINTS
// ----------------------------------------------------------------------------
int g_spawnRows[MAX_SPAWNS];
int g_spawnCols[MAX_SPAWNS];
int g_spawnCount = 0;

int g_destRows[MAX_DESTS];
int g_destCols[MAX_DESTS];
int g_destCount = 0;

// ----------------------------------------------------------------------------
// SIMULATION PARAMETERS
// ----------------------------------------------------------------------------
long g_tick = 0;
int g_seed = 0;
int g_weather = WEATHER_NORMAL;
bool g_exitRequested = false;

// ----------------------------------------------------------------------------
// METRICS
// ----------------------------------------------------------------------------
long g_totalMoves = 0;

// ----------------------------------------------------------------------------
// EMERGENCY HALT
// ----------------------------------------------------------------------------
bool g_emergencyActive = false;
int g_emergencyRow = 0;
int g_emergencyCol = 0;
int g_emergencyTimer = 0;

// ----------------------------------------------------------------------------
// QUERY HELPERS
// ----------------------------------------------------------------------------
int g_queryRow = 0;
int g_queryCol = 0;
int g_currentTrain = 0;

// ============================================================================
// INITIALIZE SIMULATION STATE
// ============================================================================
// ----------------------------------------------------------------------------
// Resets all global simulation state.
// ----------------------------------------------------------------------------
// Called before loading a new level.
// ----------------------------------------------------------------------------
void initializeSimulationState() {
    int i;
    int r;
    int c;
    g_rows = 0;
    g_cols = 0;
    for (r = 0; r < MAX_ROWS; ++r) {
        for (c = 0; c < MAX_COLS; ++c) {
            g_grid[r][c] = ' ';
        }
    }
    for (i = 0; i < (int)sizeof(g_levelName); ++i) {
        g_levelName[i] = '\0';
    }
    for (i = 0; i < (int)sizeof(g_levelPath); ++i) {
        g_levelPath[i] = '\0';
    }

    g_totalScheduledTrains = 0;
    g_activeTrainCount = 0;
    g_arrivedTrainCount = 0;
    g_crashedTrainCount = 0;
    g_tick = 0;
    g_seed = 0;
    g_weather = WEATHER_NORMAL;
    g_exitRequested = false;
    g_totalMoves = 0;

    g_spawnCount = 0;
    g_destCount = 0;
    for (i = 0; i < MAX_SPAWNS; ++i) {
        g_spawnRows[i] = 0;
        g_spawnCols[i] = 0;
    }
    for (i = 0; i < MAX_DESTS; ++i) {
        g_destRows[i] = 0;
        g_destCols[i] = 0;
    }

    for (i = 0; i < MAX_TRAINS; ++i) {
        g_trainSpawnTick[i] = 0;
        g_trainSpawnRow[i] = 0;
        g_trainSpawnCol[i] = 0;
        g_trainSpawnDir[i] = 0;
        g_trainDestIndex[i] = 0;
        g_trainRow[i] = 0;
        g_trainCol[i] = 0;
        g_trainDir[i] = 0;
        g_trainNextRow[i] = 0;
        g_trainNextCol[i] = 0;
        g_trainNextDir[i] = 0;
        g_trainSpawned[i] = false;
        g_trainActive[i] = false;
        g_trainArrived[i] = false;
        g_trainCrashed[i] = false;
        g_trainHasNext[i] = false;
    }

    for (i = 0; i < MAX_SWITCHES; ++i) {
        g_switchExists[i] = false;
        g_switchRow[i] = 0;
        g_switchCol[i] = 0;
        g_switchState[i] = 0;
        g_switchCounter[i] = 0;
        g_switchFlipQueued[i] = false;
        g_signalColor[i] = SIGNAL_GREEN;
    }

    g_emergencyActive = false;
    g_emergencyRow = 0;
    g_emergencyCol = 0;
    g_emergencyTimer = 0;

    g_queryRow = 0;
    g_queryCol = 0;
    g_currentTrain = 0;
}
