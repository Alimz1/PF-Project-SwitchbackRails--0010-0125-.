#include "switches.h"
#include "simulation_state.h"
#include "grid.h"
#include "io.h"

// ============================================================================
// SWITCHES.CPP - Switch management
// ============================================================================

// ----------------------------------------------------------------------------
// UPDATE SWITCH COUNTERS
// ----------------------------------------------------------------------------
// Increment counters for trains entering switches.
// ----------------------------------------------------------------------------
void updateSwitchCounters() {
    int i;
    for (i = 0; i < MAX_SWITCHES; ++i) {
        g_switchCounter[i] = 0;
    }
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (g_trainActive[i]) {
            g_queryRow = g_trainRow[i];
            g_queryCol = g_trainCol[i];
            if (isSwitchTile()) {
                int idx = getSwitchIndex();
                if (idx >= 0) {
                    g_switchCounter[idx]++;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// QUEUE SWITCH FLIPS
// ----------------------------------------------------------------------------
// Queue flips when counters hit K.
// ----------------------------------------------------------------------------
void queueSwitchFlips() {
    int i;
    for (i = 0; i < MAX_SWITCHES; ++i) {
        if (g_switchExists[i] && g_switchCounter[i] > 0) {
            g_switchFlipQueued[i] = true;
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY DEFERRED FLIPS
// ----------------------------------------------------------------------------
// Apply queued flips after movement.
// ----------------------------------------------------------------------------
void applyDeferredFlips() {
    int i;
    for (i = 0; i < MAX_SWITCHES; ++i) {
        if (g_switchFlipQueued[i]) {
            g_switchState[i] = 1 - g_switchState[i];
            g_switchFlipQueued[i] = false;
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE SIGNAL LIGHTS
// ----------------------------------------------------------------------------
// Update signal colors for switches.
// ----------------------------------------------------------------------------
void updateSignalLights() {
    int i;
    for (i = 0; i < MAX_SWITCHES; ++i) {
        if (!g_switchExists[i]) continue;
        if (g_switchCounter[i] == 0) {
            g_signalColor[i] = SIGNAL_GREEN;
        } else if (g_switchCounter[i] == 1) {
            g_signalColor[i] = SIGNAL_YELLOW;
        } else {
            g_signalColor[i] = SIGNAL_RED;
        }
    }
}

// ----------------------------------------------------------------------------
// TOGGLE SWITCH STATE (Manual)
// ----------------------------------------------------------------------------
// Manually toggle a switch state.
// ----------------------------------------------------------------------------
void toggleSwitchState() {
    if (!isSwitchTile()) return;
    int idx = getSwitchIndex();
    if (idx >= 0 && g_switchExists[idx]) {
        g_switchState[idx] = 1 - g_switchState[idx];
    }
}

// ----------------------------------------------------------------------------
// GET SWITCH STATE FOR DIRECTION
// ----------------------------------------------------------------------------
// Return the state for a given direction.
// ----------------------------------------------------------------------------
int getSwitchStateForDirection() {
    if (!isSwitchTile()) return 0;
    int idx = getSwitchIndex();
    if (idx < 0 || !g_switchExists[idx]) return 0;
    return g_switchState[idx];
}
