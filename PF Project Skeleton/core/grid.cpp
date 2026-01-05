#include "grid.h"
#include "simulation_state.h"

// ============================================================================
// GRID.CPP - Grid utilities
// ============================================================================

// Helper to read the tile at the queried position safely.
static char getTileAtQuery() {
    if (g_queryRow < 0 || g_queryRow >= g_rows || g_queryCol < 0 || g_queryCol >= g_cols) {
        return ' ';
    }
    return g_grid[g_queryRow][g_queryCol];
}

// ----------------------------------------------------------------------------
// Check if a position is inside the grid.
// ----------------------------------------------------------------------------
// Returns true if x,y are within bounds.
// ----------------------------------------------------------------------------
bool isInBounds() {
    return g_queryRow >= 0 && g_queryRow < g_rows && g_queryCol >= 0 && g_queryCol < g_cols;
}

// ----------------------------------------------------------------------------
// Check if a tile is a track tile.
// ----------------------------------------------------------------------------
// Returns true if the tile can be traversed by trains.
// ----------------------------------------------------------------------------
bool isTrackTile() {
    char tile = getTileAtQuery();
    if (tile == '=' || tile == '|' || tile == '+' || tile == 'S' || tile == 'D' || tile == '#') {
        return true;
    }
    if (tile >= 'A' && tile <= 'Z') {
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// Check if a tile is a switch.
// ----------------------------------------------------------------------------
// Returns true if the tile is 'A'..'Z'.
// ----------------------------------------------------------------------------
bool isSwitchTile() {
    char tile = getTileAtQuery();
    return tile >= 'A' && tile <= 'Z';
}

// ----------------------------------------------------------------------------
// Get switch index from character.
// ----------------------------------------------------------------------------
// Maps 'A'..'Z' to 0..25, else -1.
// ----------------------------------------------------------------------------
int getSwitchIndex() {
    char tile = getTileAtQuery();
    if (tile >= 'A' && tile <= 'Z') {
        return tile - 'A';
    }
    return -1;
}

// ----------------------------------------------------------------------------
// Check if a position is a spawn point.
// ----------------------------------------------------------------------------
// Returns true if x,y is a spawn.
// ----------------------------------------------------------------------------
bool isSpawnPoint() {
    char tile = getTileAtQuery();
    return tile == 'S';
}

// ----------------------------------------------------------------------------
// Check if a position is a destination.
// ----------------------------------------------------------------------------
// Returns true if x,y is a destination.
// ----------------------------------------------------------------------------
bool isDestinationPoint() {
    char tile = getTileAtQuery();
    return tile == 'D';
}

// ----------------------------------------------------------------------------
// Toggle a safety tile.
// ----------------------------------------------------------------------------
// Returns true if toggled successfully.
// ----------------------------------------------------------------------------
bool toggleSafetyTile() {
    if (!isInBounds()) {
        return false;
    }
    char tile = getTileAtQuery();
    if (tile == '#') {
        g_grid[g_queryRow][g_queryCol] = ' ';
        return true;
    }
    if (tile == ' ') {
        g_grid[g_queryRow][g_queryCol] = '#';
        return true;
    }
    return false;
}
