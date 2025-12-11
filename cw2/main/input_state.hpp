#ifndef INPUT_STATE_HPP
#define INPUT_STATE_HPP

struct InputState
{
    // Movement flags
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;

    // Mouse control
    bool mouseLookActive = false;
    bool firstMouse = true;
    float lastMouseX = 0.f;
    float lastMouseY = 0.f;

    // Key modifiers
    bool shiftPressed = false;
    bool controlPressed = false;

    // Methods
    // Reset all input state
    void resetAll() noexcept {
        resetMovement();
        resetMouse();
        resetModifiers();
        mouseLookActive = false;
    }

    void resetMovement() noexcept
    {
        moveForward = false;
        moveBackward = false;
        moveLeft = false;
        moveRight = false;
        moveUp = false;
        moveDown = false;
    }

    void resetMouse() noexcept
    {
        firstMouse = true;
        lastMouseX = 0.f;
        lastMouseY = 0.f;
    }

    void resetModifiers() noexcept
    {
        shiftPressed = false;
        controlPressed = false;
    }

    void updateMousePosition(float currentX, float currentY) noexcept
    {
        lastMouseX = currentX;
        lastMouseY = currentY;
    }

    void activateMouseLook(float currentX, float currentY) noexcept
    {
        mouseLookActive = true;
        lastMouseX = currentX;
        lastMouseY = currentY;
        firstMouse = true;
    }

    void deactivateMouseLook() noexcept
    {
        mouseLookActive = false;
        resetMouse();
    }

    // Check if any movement key is pressed
    bool isMoving() const noexcept
    {
        return moveForward || moveBackward || moveLeft || moveRight || moveUp || moveDown;
    }
};

#endif // INPUT_STATE_HPP