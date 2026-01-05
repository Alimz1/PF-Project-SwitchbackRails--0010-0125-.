#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include <iostream>

using namespace std;

// ============================================================================
// MAIN.CPP - Entry point of the application (NO CLASSES)
// ============================================================================

// ----------------------------------------------------------------------------
// MAIN ENTRY POINT
// ----------------------------------------------------------------------------
int main(int argc, char** argv) {
    const char* defaultLevel = "data/levels/easy_level.lvl";
    initializeSimulationState();

    if (argc > 1) {
        int i;
        for (i = 0; i < (int)sizeof(g_levelPath) - 1 && argv[1][i] != '\0'; ++i) {
            g_levelPath[i] = argv[1][i];
        }
        g_levelPath[i] = '\0';
    } else {
        int i;
        for (i = 0; i < (int)sizeof(g_levelPath) - 1 && defaultLevel[i] != '\0'; ++i) {
            g_levelPath[i] = defaultLevel[i];
        }
        g_levelPath[i] = '\0';
    }

    if (!loadLevelFile()) {
        cerr << "Failed to load level file: " << g_levelPath << endl;
        return 1;
    }

    initializeSimulation();

    cout << "Switchback Rails\n";
    cout << "Loaded level: " << g_levelName << "\n";
    cout << "Controls: SPACE pause/resume, . step, mouse to edit, ESC quit\n";

    if (!initializeApp()) {
        cerr << "Failed to initialize window.\n";
        return 1;
    }

    runApp();
    writeMetrics();
    cleanupApp();
    return 0;
}
