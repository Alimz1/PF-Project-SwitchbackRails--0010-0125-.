#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"
#include <cstdlib>
#include <cmath>

using namespace std;

// ============================================================================
// TRAINS.CPP - Train logic
// ============================================================================

// Direction helpers
static const int dRow[4] = {-1, 0, 1, 0};
static const int dCol[4] = {0, 1, 0, -1};

// ----------------------------------------------------------------------------
// Utility: check if a cell is walkable.
// ----------------------------------------------------------------------------
static bool isWalkable(int row, int col) {
    g_queryRow = row;
    g_queryCol = col;
    return isInBounds() && isTrackTile();
}

// ----------------------------------------------------------------------------
// Utility: is the position occupied by another active train?
// ----------------------------------------------------------------------------
static bool isOccupied(int row, int col, int ignoreTrain) {
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (i == ignoreTrain) continue;
        if (g_trainActive[i] && !g_trainCrashed[i]) {
            if (g_trainRow[i] == row && g_trainCol[i] == col) {
                return true;
            }
        }
    }
    return false;
}

// ----------------------------------------------------------------------------
// SPAWN TRAINS FOR CURRENT TICK
// ----------------------------------------------------------------------------
// Activate trains scheduled for this tick.
// ----------------------------------------------------------------------------
void spawnTrainsForTick() {
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (!g_trainSpawned[i] && g_trainSpawnTick[i] <= g_tick) {
            int row = g_trainSpawnRow[i];
            int col = g_trainSpawnCol[i];
            if (!isOccupied(row, col, -1)) {
                g_trainRow[i] = row;
                g_trainCol[i] = col;
                g_trainDir[i] = g_trainSpawnDir[i] % 4;
                g_trainSpawned[i] = true;
                g_trainActive[i] = true;
                g_activeTrainCount++;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Choose best direction toward destination.
// ----------------------------------------------------------------------------
static int chooseDirectionByDistance(int row, int col, int destIndex, int currentDir) {
    int bestDir = currentDir;
    int bestDist = 1000000;
    int dir;
    for (dir = 0; dir < 4; ++dir) {
        int nr = row + dRow[dir];
        int nc = col + dCol[dir];
        if (!isWalkable(nr, nc)) continue;
        int dist = 0;
        if (destIndex >= 0 && destIndex < g_destCount) {
            int dr = g_destRows[destIndex] - nr;
            if (dr < 0) dr = -dr;
            int dc = g_destCols[destIndex] - nc;
            if (dc < 0) dc = -dc;
            dist = dr + dc;
        }
        if (dist < bestDist) {
            bestDist = dist;
            bestDir = dir;
        }
    }
    return bestDir;
}

// ----------------------------------------------------------------------------
// DETERMINE NEXT POSITION for a train
// ----------------------------------------------------------------------------
// Compute next position/direction from current tile and rules.
// ----------------------------------------------------------------------------
bool determineNextPosition() {
    int idx = g_currentTrain;
    if (!g_trainActive[idx] || g_trainCrashed[idx]) {
        return false;
    }

    int row = g_trainRow[idx];
    int col = g_trainCol[idx];
    int dir = g_trainDir[idx];
    int destIndex = g_trainDestIndex[idx];

    g_queryRow = row;
    g_queryCol = col;
    char tile = g_grid[row][col];

    int nextDir = dir;

    if (tile == '+') {
        nextDir = getSmartDirectionAtCrossing();
    } else if (tile >= 'A' && tile <= 'Z') {
        int state = getSwitchStateForDirection();
        if (dir == DIR_LEFT || dir == DIR_RIGHT) {
            if (state == 0 && isWalkable(row, col + dCol[dir])) {
                nextDir = dir;
            } else if (isWalkable(row + 1, col)) {
                nextDir = DIR_DOWN;
            } else if (isWalkable(row - 1, col)) {
                nextDir = DIR_UP;
            }
        } else {
            if (state == 0 && isWalkable(row + dRow[dir], col)) {
                nextDir = dir;
            } else if (isWalkable(row, col + 1)) {
                nextDir = DIR_RIGHT;
            } else if (isWalkable(row, col - 1)) {
                nextDir = DIR_LEFT;
            }
        }
    } else if (tile == '=' || tile == '|') {
        int nr = row + dRow[dir];
        int nc = col + dCol[dir];
        if (!isWalkable(nr, nc)) {
            nextDir = chooseDirectionByDistance(row, col, destIndex, dir);
        }
    } else {
        nextDir = chooseDirectionByDistance(row, col, destIndex, dir);
    }

    int nextRow = row + dRow[nextDir];
    int nextCol = col + dCol[nextDir];

    g_trainNextRow[idx] = nextRow;
    g_trainNextCol[idx] = nextCol;
    g_trainNextDir[idx] = nextDir;
    g_trainHasNext[idx] = true;

    g_queryRow = nextRow;
    g_queryCol = nextCol;
    if (!isInBounds() || !isTrackTile()) {
        g_trainHasNext[idx] = false;
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
// GET NEXT DIRECTION based on current tile and direction
// ----------------------------------------------------------------------------
// Return new direction after entering the tile.
// ----------------------------------------------------------------------------
int getNextDirection() {
    int idx = g_currentTrain;
    if (!g_trainHasNext[idx]) return g_trainDir[idx];
    return g_trainNextDir[idx];
}

// ----------------------------------------------------------------------------
// SMART ROUTING AT CROSSING - Route train to its matched destination
// ----------------------------------------------------------------------------
// Choose best direction at '+' toward destination.
// ----------------------------------------------------------------------------
int getSmartDirectionAtCrossing() {
    int idx = g_currentTrain;
    return chooseDirectionByDistance(g_trainRow[idx], g_trainCol[idx], g_trainDestIndex[idx], g_trainDir[idx]);
}

// ----------------------------------------------------------------------------
// DETERMINE ALL ROUTES (PHASE 2)
// ----------------------------------------------------------------------------
// Fill next positions/directions for all trains.
// ----------------------------------------------------------------------------
void determineAllRoutes() {
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        g_trainHasNext[i] = false;
    }
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (g_trainActive[i] && !g_trainCrashed[i]) {
            g_currentTrain = i;
            determineNextPosition();
        }
    }
}

// ----------------------------------------------------------------------------
// DETECT COLLISIONS WITH PRIORITY SYSTEM
// ----------------------------------------------------------------------------
// Resolve same-tile, swap, and crossing conflicts.
// ----------------------------------------------------------------------------
void detectCollisions() {
    int i, j;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (!g_trainActive[i] || !g_trainHasNext[i]) continue;
        for (j = i + 1; j < g_totalScheduledTrains; ++j) {
            if (!g_trainActive[j] || !g_trainHasNext[j]) continue;

            bool sameTarget = (g_trainNextRow[i] == g_trainNextRow[j] && g_trainNextCol[i] == g_trainNextCol[j]);
            bool swapTarget = (g_trainNextRow[i] == g_trainRow[j] && g_trainNextCol[i] == g_trainCol[j] &&
                               g_trainNextRow[j] == g_trainRow[i] && g_trainNextCol[j] == g_trainCol[i]);

            if (sameTarget || swapTarget) {
                g_trainCrashed[i] = true;
                g_trainCrashed[j] = true;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// MOVE ALL TRAINS (PHASE 5)
// ----------------------------------------------------------------------------
// Move trains; resolve collisions and apply effects.
// ----------------------------------------------------------------------------
void moveAllTrains() {
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (!g_trainActive[i]) continue;
        if (g_trainCrashed[i]) {
            g_trainActive[i] = false;
            g_crashedTrainCount++;
            g_activeTrainCount--;
            continue;
        }
        if (!g_trainHasNext[i]) {
            g_trainCrashed[i] = true;
            g_trainActive[i] = false;
            g_crashedTrainCount++;
            g_activeTrainCount--;
            continue;
        }
        g_trainRow[i] = g_trainNextRow[i];
        g_trainCol[i] = g_trainNextCol[i];
        g_trainDir[i] = g_trainNextDir[i];
        g_totalMoves++;
    }
}

// ----------------------------------------------------------------------------
// CHECK ARRIVALS
// ----------------------------------------------------------------------------
// Mark trains that reached destinations.
// ----------------------------------------------------------------------------
void checkArrivals() {
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (!g_trainActive[i]) continue;
        int destIndex = g_trainDestIndex[i];
        if (destIndex >= 0 && destIndex < g_destCount) {
            if (g_trainRow[i] == g_destRows[destIndex] && g_trainCol[i] == g_destCols[destIndex]) {
                g_trainActive[i] = false;
                g_trainArrived[i] = true;
                g_arrivedTrainCount++;
                g_activeTrainCount--;
            }
        } else {
            g_queryRow = g_trainRow[i];
            g_queryCol = g_trainCol[i];
            if (isDestinationPoint()) {
                g_trainActive[i] = false;
                g_trainArrived[i] = true;
                g_arrivedTrainCount++;
                g_activeTrainCount--;
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY EMERGENCY HALT
// ----------------------------------------------------------------------------
// Apply halt to trains in the active zone.
// ----------------------------------------------------------------------------
void applyEmergencyHalt() {
    if (!g_emergencyActive) return;
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (!g_trainActive[i]) continue;
        int dr = g_trainRow[i] - g_emergencyRow;
        if (dr < 0) dr = -dr;
        int dc = g_trainCol[i] - g_emergencyCol;
        if (dc < 0) dc = -dc;
        if (dr <= 1 && dc <= 1) {
            g_trainHasNext[i] = false;
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE EMERGENCY HALT
// ----------------------------------------------------------------------------
// Decrement timer and disable when done.
// ----------------------------------------------------------------------------
void updateEmergencyHalt() {
    if (!g_emergencyActive) return;
    g_emergencyTimer--;
    if (g_emergencyTimer <= 0) {
        g_emergencyActive = false;
    }
}
