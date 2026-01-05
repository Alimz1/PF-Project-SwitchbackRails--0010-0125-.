#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/grid.h"
#include "../core/switches.h"
#include "../core/io.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdio>

// ============================================================================
// APP.CPP - Implementation of SFML application (NO CLASSES)
// ============================================================================

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES FOR APP STATE
// ----------------------------------------------------------------------------
static sf::RenderWindow* g_window = nullptr;
static sf::Font g_font;

// View for camera (panning/zoom)
static sf::View g_camera;

// Simulation state
static bool g_isPaused = false;
static bool g_isStepMode = false;

// Mouse state
static bool g_isDragging = false;
static int g_lastMouseX = 0;
static int g_lastMouseY = 0;

// Grid rendering parameters
static float g_cellSize = 32.0f;
static float g_gridOffsetX = 50.0f;
static float g_gridOffsetY = 50.0f;

// Helper to convert mouse to grid coordinates.
static bool mouseToGrid(int mouseX, int mouseY, int* outRow, int* outCol) {
    float worldX = (float)mouseX;
    float worldY = (float)mouseY;
    worldX += g_camera.getCenter().x - (g_camera.getSize().x / 2.0f);
    worldY += g_camera.getCenter().y - (g_camera.getSize().y / 2.0f);

    worldX -= g_gridOffsetX;
    worldY -= g_gridOffsetY;
    if (worldX < 0 || worldY < 0) return false;
    int col = (int)(worldX / g_cellSize);
    int row = (int)(worldY / g_cellSize);
    if (row < 0 || row >= g_rows || col < 0 || col >= g_cols) return false;
    *outRow = row;
    *outCol = col;
    return true;
}

// Helper to draw the grid and trains.
static void drawScene() {
    sf::RectangleShape cell(sf::Vector2f(g_cellSize - 2, g_cellSize - 2));
    int r, c;
    for (r = 0; r < g_rows; ++r) {
        for (c = 0; c < g_cols; ++c) {
            char tile = g_grid[r][c];
            sf::Color color(30, 30, 30);
            if (tile == '=' || tile == '|') color = sf::Color(150, 150, 150);
            else if (tile == '+') color = sf::Color(170, 170, 220);
            else if (tile == 'S') color = sf::Color(70, 180, 70);
            else if (tile == 'D') color = sf::Color(70, 130, 200);
            else if (tile >= 'A' && tile <= 'Z') color = sf::Color(220, 190, 70);
            else if (tile == '#') color = sf::Color(200, 120, 200);
            cell.setFillColor(color);
            cell.setPosition(g_gridOffsetX + c * g_cellSize, g_gridOffsetY + r * g_cellSize);
            g_window->draw(cell);
        }
    }

    sf::CircleShape trainShape(g_cellSize / 2.5f);
    trainShape.setOrigin(g_cellSize / 2.5f, g_cellSize / 2.5f);
    for (r = 0; r < g_totalScheduledTrains; ++r) {
        if (!g_trainActive[r]) continue;
        float x = g_gridOffsetX + g_trainCol[r] * g_cellSize + g_cellSize / 2.0f;
        float y = g_gridOffsetY + g_trainRow[r] * g_cellSize + g_cellSize / 2.0f;
        trainShape.setPosition(x, y);
        trainShape.setFillColor(sf::Color(220, 80, 80));
        g_window->draw(trainShape);
    }
}

// ----------------------------------------------------------------------------
// INITIALIZATION
// ----------------------------------------------------------------------------
bool initializeApp() {
    if (g_window != nullptr) return true;
    g_window = new sf::RenderWindow(sf::VideoMode(1200, 800), "Switchback Rails", sf::Style::Close);
    g_window->setFramerateLimit(60);
    g_camera = g_window->getDefaultView();
    g_camera.setCenter(600.0f, 400.0f);
    return true;
}

// ----------------------------------------------------------------------------
// MAIN RUN LOOP
// ----------------------------------------------------------------------------
void runApp() {
    sf::Clock clock;
    float accumulator = 0.0f;
    const float tickTime = 0.5f;

    while (g_window->isOpen() && !g_exitRequested) {
        sf::Event event;
        while (g_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                g_window->close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    g_isPaused = !g_isPaused;
                } else if (event.key.code == sf::Keyboard::Period) {
                    g_isStepMode = true;
                } else if (event.key.code == sf::Keyboard::Escape) {
                    g_exitRequested = true;
                    g_window->close();
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) g_cellSize += 2.0f;
                else if (g_cellSize > 10.0f) g_cellSize -= 2.0f;
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    g_isDragging = true;
                    g_lastMouseX = event.mouseButton.x;
                    g_lastMouseY = event.mouseButton.y;
                } else if (event.mouseButton.button == sf::Mouse::Left) {
                    int row, col;
                    if (mouseToGrid(event.mouseButton.x, event.mouseButton.y, &row, &col)) {
                        g_queryRow = row;
                        g_queryCol = col;
                        toggleSafetyTile();
                    }
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    int row, col;
                    if (mouseToGrid(event.mouseButton.x, event.mouseButton.y, &row, &col)) {
                        g_queryRow = row;
                        g_queryCol = col;
                        toggleSwitchState();
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    g_isDragging = false;
                }
            } else if (event.type == sf::Event::MouseMoved && g_isDragging) {
                int dx = event.mouseMove.x - g_lastMouseX;
                int dy = event.mouseMove.y - g_lastMouseY;
                g_lastMouseX = event.mouseMove.x;
                g_lastMouseY = event.mouseMove.y;
                g_camera.move(-(float)dx, -(float)dy);
            }
        }

        float dt = clock.restart().asSeconds();
        accumulator += dt;
        if (!g_isPaused) {
            if (g_isStepMode) {
                simulateOneTick();
                g_isStepMode = false;
            } else if (accumulator >= tickTime) {
                simulateOneTick();
                accumulator = 0.0f;
            }
        }

        if (isSimulationComplete()) {
            g_isPaused = true;
        }

        g_window->setView(g_camera);
        g_window->clear(sf::Color(20, 20, 20));
        drawScene();
        g_window->display();
    }
}

// ----------------------------------------------------------------------------
// CLEANUP
// ----------------------------------------------------------------------------
void cleanupApp() {
    if (g_window) {
        g_window->close();
        delete g_window;
        g_window = nullptr;
    }
}
