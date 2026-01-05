#include "io.h"
#include "simulation_state.h"
#include "grid.h"
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

using namespace std;

// ============================================================================
// IO.CPP - Level I/O and logging
// ============================================================================

// Helper: trim whitespace from both ends of a string.
static string trim(const string& value) {
    size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t' || value[start] == '\r')) {
        start++;
    }
    size_t end = value.size();
    while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r')) {
        end--;
    }
    return value.substr(start, end - start);
}

// Helper: record spawn/destination positions while reading the map.
static void recordSpecialTile(int row, int col, char tile) {
    if (tile == 'S' && g_spawnCount < MAX_SPAWNS) {
        g_spawnRows[g_spawnCount] = row;
        g_spawnCols[g_spawnCount] = col;
        g_spawnCount++;
    } else if (tile == 'D' && g_destCount < MAX_DESTS) {
        g_destRows[g_destCount] = row;
        g_destCols[g_destCount] = col;
        g_destCount++;
    } else if (tile >= 'A' && tile <= 'Z') {
        int idx = tile - 'A';
        g_switchExists[idx] = true;
        g_switchRow[idx] = row;
        g_switchCol[idx] = col;
    }
}

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
// Load a .lvl file into global state.
// ----------------------------------------------------------------------------
bool loadLevelFile() {
    ifstream in(g_levelPath);
    if (!in.is_open()) {
        return false;
    }

    string line;
    string section = "";
    int mapRow = 0;
    int trainIndex = 0;

    while (getline(in, line)) {
        if (line.size() == 0) {
            continue;
        }
        if (line.back() == '\n') {
            line.pop_back();
        }
        int colonIndex = -1;
        int i;
        for (i = 0; i < (int)line.size(); ++i) {
            if (line[i] == ':') {
                colonIndex = i;
            }
        }
        if (colonIndex == (int)line.size() - 1) {
            section = trim(line.substr(0, line.size() - 1));
            continue;
        }

        if (section == "NAME") {
            string nameText = trim(line);
            for (i = 0; i < (int)sizeof(g_levelName) - 1 && i < (int)nameText.size(); ++i) {
                g_levelName[i] = nameText[i];
            }
            if (i < (int)sizeof(g_levelName)) {
                g_levelName[i] = '\0';
            } else {
                g_levelName[sizeof(g_levelName) - 1] = '\0';
            }
        } else if (section == "ROWS") {
            g_rows = atoi(line.c_str());
        } else if (section == "COLS") {
            g_cols = atoi(line.c_str());
        } else if (section == "SEED") {
            g_seed = atoi(line.c_str());
            srand(g_seed);
        } else if (section == "WEATHER") {
            string weatherText = trim(line);
            if (weatherText == "RAIN") g_weather = WEATHER_RAIN;
            else if (weatherText == "FOG") g_weather = WEATHER_FOG;
            else g_weather = WEATHER_NORMAL;
        } else if (section == "MAP") {
            if (mapRow < g_rows) {
                int c;
                for (c = 0; c < g_cols; ++c) {
                    char tile = ' ';
                    if (c < (int)line.size()) {
                        tile = line[c];
                    }
                    g_grid[mapRow][c] = tile;
                    recordSpecialTile(mapRow, c, tile);
                }
                mapRow++;
            }
        } else if (section == "SWITCHES") {
            if (line.size() > 0) {
                stringstream ss(line);
                char name;
                ss >> name;
                if (name >= 'A' && name <= 'Z') {
                    int idx = name - 'A';
                    string mode;
                    string rest;
                    ss >> mode;
                    g_switchExists[idx] = true;
                    g_switchState[idx] = 0;
                    while (ss >> rest) {
                        if (rest == "TURN") {
                            g_switchState[idx] = 1;
                        }
                    }
                }
            }
        } else if (section == "TRAINS") {
            if (trainIndex < MAX_TRAINS) {
                int spawnTick, row, col, dir, destIdx;
                if (sscanf(line.c_str(), "%d %d %d %d %d", &spawnTick, &row, &col, &dir, &destIdx) == 5) {
                    g_trainSpawnTick[trainIndex] = spawnTick;
                    g_trainSpawnRow[trainIndex] = row;
                    g_trainSpawnCol[trainIndex] = col;
                    g_trainSpawnDir[trainIndex] = dir;
                    g_trainDestIndex[trainIndex] = destIdx;
                    g_trainSpawned[trainIndex] = false;
                    trainIndex++;
                }
            }
        }
    }

    g_totalScheduledTrains = trainIndex;
    return g_rows > 0 && g_cols > 0;
}

// ----------------------------------------------------------------------------
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
// Create/clear CSV logs with headers.
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    system("mkdir -p \"out\"");
    ofstream trace("out/trace.csv", ios::out | ios::trunc);
    trace << "tick,train,row,col,dir,state\n";
    trace.close();

    ofstream sw("out/switches.csv", ios::out | ios::trunc);
    sw << "tick,switch,state,signal\n";
    sw.close();

    ofstream sig("out/signals.csv", ios::out | ios::trunc);
    sig << "tick,switch,color\n";
    sig.close();

    ofstream metrics("out/metrics.txt", ios::out | ios::trunc);
    metrics << "Switchback Rails Metrics\n";
    metrics.close();
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
// Append tick, train id, position, direction, state to trace.csv.
// ----------------------------------------------------------------------------
void logTrainTrace() {
    ofstream trace("out/trace.csv", ios::out | ios::app);
    int i;
    for (i = 0; i < g_totalScheduledTrains; ++i) {
        if (g_trainActive[i] || g_trainArrived[i] || g_trainCrashed[i]) {
            trace << g_tick << "," << i << "," << g_trainRow[i] << "," << g_trainCol[i] << ",";
            trace << g_trainDir[i] << ",";
            if (g_trainArrived[i]) trace << "ARRIVED\n";
            else if (g_trainCrashed[i]) trace << "CRASHED\n";
            else trace << "RUNNING\n";
        }
    }
    trace.close();
}

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
// Append tick, switch id/mode/state to switches.csv.
// ----------------------------------------------------------------------------
void logSwitchState() {
    ofstream sw("out/switches.csv", ios::out | ios::app);
    int i;
    for (i = 0; i < MAX_SWITCHES; ++i) {
        if (g_switchExists[i]) {
            sw << g_tick << "," << (char)('A' + i) << "," << g_switchState[i] << "," << g_signalColor[i] << "\n";
        }
    }
    sw.close();
}

// ----------------------------------------------------------------------------
// LOG SIGNAL STATE
// ----------------------------------------------------------------------------
// Append tick, switch id, signal color to signals.csv.
// ----------------------------------------------------------------------------
void logSignalState() {
    ofstream sig("out/signals.csv", ios::out | ios::app);
    int i;
    for (i = 0; i < MAX_SWITCHES; ++i) {
        if (g_switchExists[i]) {
            sig << g_tick << "," << (char)('A' + i) << "," << g_signalColor[i] << "\n";
        }
    }
    sig.close();
}

// ----------------------------------------------------------------------------
// WRITE FINAL METRICS
// ----------------------------------------------------------------------------
// Write summary metrics to metrics.txt.
// ----------------------------------------------------------------------------
void writeMetrics() {
    ofstream metrics("out/metrics.txt", ios::out | ios::app);
    metrics << "Ticks: " << g_tick << "\n";
    metrics << "Trains spawned: " << g_totalScheduledTrains << "\n";
    metrics << "Trains arrived: " << g_arrivedTrainCount << "\n";
    metrics << "Trains crashed: " << g_crashedTrainCount << "\n";
    metrics << "Moves: " << g_totalMoves << "\n";
    metrics.close();
}
