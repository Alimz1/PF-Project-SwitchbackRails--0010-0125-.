#include "simulation.h"
#include "simulation_state.h"
#include "trains.h"
#include "switches.h"
#include "io.h"
#include <cstdlib>
#include <ctime>

// ============================================================================
// SIMULATION.CPP - Implementation of main simulation logic
// ============================================================================

// ----------------------------------------------------------------------------
// INITIALIZE SIMULATION
// ----------------------------------------------------------------------------
void initializeSimulation() {
    initializeLogFiles();
    g_tick = 0;
    g_totalMoves = 0;
}

// ----------------------------------------------------------------------------
// SIMULATE ONE TICK
// ----------------------------------------------------------------------------
void simulateOneTick() {
    spawnTrainsForTick();
    determineAllRoutes();
    applyEmergencyHalt();
    detectCollisions();
    moveAllTrains();
    checkArrivals();
    updateSwitchCounters();
    queueSwitchFlips();
    applyDeferredFlips();
    updateSignalLights();
    logTrainTrace();
    logSwitchState();
    logSignalState();
    updateEmergencyHalt();
    g_tick++;
}

// ----------------------------------------------------------------------------
// CHECK IF SIMULATION IS COMPLETE
// ----------------------------------------------------------------------------
bool isSimulationComplete() {
    int finished = g_arrivedTrainCount + g_crashedTrainCount;
    bool allSpawned = true;
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (!g_trainSpawned[i]) {
            allSpawned = false;
            break;
        }
    }
    return allSpawned && finished >= g_totalScheduledTrains && g_activeTrainCount == 0;
}
