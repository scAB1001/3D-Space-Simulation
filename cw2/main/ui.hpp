#ifndef UI_HPP
#define UI_HPP

#include "state.hpp"

// Draws all 2D UI elements (Task 1.11)
// Must be called AFTER 3D rendering

void renderUI(State_& state,
              float fbWidth,
              float fbHeight,
              GLuint uiShader);
#endif // UI_HPP
