#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <print>
#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

// Definitions
#include "defaults.hpp"
#include "config.hpp"
#include "camera.hpp"
#include "input_state.hpp"
#include "animation_state.hpp"
#include "renderer.hpp"

// Shapes
#include "simple_mesh.hpp"
#include "cone.hpp"
#include "cylinder.hpp"

// Utilities
#include "landing_pad.hpp"
#include "texture.hpp"
#include "loadobj.hpp"
#include "test.hpp"

// TODO: LIST
// - 1.4 CALL DRAW_ARRAY TWICE
// - 1.6 BILL-PHONG GOES IN FRAG ONLY
// - 1.7 USE AN EQUATION THAT IS A FUNCTION T, AS T INCREASES, SLOW ACCELERATE FROM STANDSTILL
// - 1.10 DRAW AS MANY AS POSSIBLE WITHOUT LOSING FPS (60 MAX for ~1,000,000), VARIABLE (LIKE T particles)

namespace
{
	constexpr char const *kWindowTitle = "COMP3811 - CW2";

	struct State_ {
        ShaderProgram* prog = nullptr;
        Camera camera;
        InputState input;
		AnimationState animation;
	};

	// GLFW Callbacks Declarations
	void glfw_callback_error_(int, char const *);
	void glfw_callback_key_(GLFWwindow *, int, int, int, int);
	void glfw_callback_mouse_button_(GLFWwindow *, int, int, int);
	void glfw_callback_motion_(GLFWwindow *, double, double);

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}


int main()
try
{
	// Initialize GLFW
	if (GLFW_TRUE != glfwInit())
	{
		char const *msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwInit() failed with '{}' ({})", msg, ecode);
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback(&glfw_callback_error_);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

#if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // ~ !NDEBUG

	GLFWwindow *window = glfwCreateWindow(1280, 720, kWindowTitle, nullptr, nullptr);
	if (!window)
	{
		char const *msg = nullptr;
		int ecode = glfwGetError(&msg);
		throw Error("glfwCreateWindow() failed with '{}' ({})", msg, ecode);
	}

	GLFWWindowDeleter windowDeleter{window};

	// Set up event handling
	State_ state{};
	glfwSetWindowUserPointer(window, &state);
	glfwSetKeyCallback(window, &glfw_callback_key_);
	glfwSetMouseButtonCallback(window, &glfw_callback_mouse_button_);
	glfwSetCursorPosCallback(window, &glfw_callback_motion_);

	// Set up drawing stuff
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if (!gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress))
		throw Error("gladLoadGLLoader() failed - cannot load GL API!");

	std::print("RENDERER {}\n", (char const *)glGetString(GL_RENDERER));
	std::print("VENDOR {}\n", (char const *)glGetString(GL_VENDOR));
	std::print("VERSION {}\n", (char const *)glGetString(GL_VERSION));
	std::print("SHADING_LANGUAGE_VERSION {}\n", (char const *)glGetString(GL_SHADING_LANGUAGE_VERSION));


	// Ddebug output
#if !defined(NDEBUG)
	setup_gl_debug_output();
#endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: Global GL setup goes here
	globalGLSetup();

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize(window, &iwidth, &iheight);
	glViewport(0, 0, iwidth, iheight);

	/* Important Note: There's a trick here.
	 * When we have texcoords, we're storing them as vec2 in the buffer,
	 * 	but the shader reads them as vec3 at location 1.
	 * This works because:
	 * 	Texcoords: vec2(x, y) in buffer -> read as vec3(x, y, ?) in shader
	 * 	The z-component is ignored for texcoords
	 * For colors: vec3(r, g, b) in buffer -> read as vec3(r, g, b) in shader
	 */
	ShaderProgram unifiedProg( {
		{ GL_VERTEX_SHADER, "assets/cw2/shaders/u.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/shaders/u.frag" }
	} );
	state.prog = &unifiedProg;

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;

	//TODO: create VBOs and VAO
	auto parlahtiMesh = load_wavefront_obj("assets/cw2/parlahti.obj");
	GLuint parlahtiVao = create_vao(parlahtiMesh);
	std::size_t parlahtiVertexCount = parlahtiMesh.vertexCount();
	std::println("Terrain loaded: {} vertices, material type: {}",
                 parlahtiVertexCount, parlahtiMesh.materialType);

	GLuint parlahtiTexture = 0;
	if (parlahtiMesh.materialType == 1 && parlahtiMesh.hasTexcoords())
	{
		std::println( "Loading parlahti texture..." );
		parlahtiTexture = load_texture_2d("assets/cw2/L4343A-4k.jpeg");
	}

	std::vector<LandingPad> landingPads = {
		LandingPad(Config::World::kLandingPad1Pos, Config::World::kLandingPadScale),
		LandingPad(Config::World::kLandingPad2Pos, Config::World::kLandingPadScale)
	};

	/* CUBE */
	// auto cubeMesh = make_cube_with_normals({0.8f, 0.2f, 0.2f});
	auto cubeMesh = make_indexed_cube({0.8f, 0.2f, 0.2f});
	cubeMesh.materialType = 0; // Colored
	GLuint cubeVao = create_vao(cubeMesh);
	std::size_t cubeVertexCount = cubeMesh.vertexCount();

	// -------------- Run tests --------------
	// test_all_mesh_functions();
	// test_vao_creation();

	// Reset state. This unbinds any VAO or VBO we may have left bound.
	// Unbind ebo too (to be safe), do this by calling
	resetBindings();

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Let GLFW process events
		glfwPollEvents();

		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize(window, &nwidth, &nheight);

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if (0 == nwidth || 0 == nheight)
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize(window, &nwidth, &nheight);
				} while (0 == nwidth || 0 == nheight);
			}

			glViewport(0, 0, nwidth, nheight);
		}
		float aspectRatio = static_cast<float>(fbwidth) / static_cast<float>(fbheight);

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;

		angle += dt * Config::kFloatPi * 0.3f;
		if (angle >= 2.f * Config::kFloatPi)
			angle -= 2.f * Config::kFloatPi;

		// Update camera state
		state.camera.updateVectors();

		// Handle camera movement in free mode only
		if (state.input.mouseLookActive && state.camera.isMode(Camera::Mode::Free))
		{
			if (state.input.moveForward) state.camera.moveForward(dt);
			if (state.input.moveBackward) state.camera.moveBackward(dt);
			if (state.input.moveLeft) state.camera.moveLeft(dt);
			if (state.input.moveRight) state.camera.moveRight(dt);
			if (state.input.moveUp) state.camera.moveUp(dt);
			if (state.input.moveDown) state.camera.moveDown(dt);

			// Clamp position
			state.camera.clampVertical(Config::World::kMinCameraHeight, Config::World::kMaxCameraHeight);
			state.camera.clampToWorldBounds();
		}

		// Update animation state
		if (state.animation.isAnimating && !state.animation.isPaused)
		{
			state.animation.update(dt);

			state.camera.updateForAnimation(state.animation.currentPosition,
											state.animation.velocity, dt);

			// Periodic debug output
			static float lastDebugTime = 0.0f;
			state.animation.printDebugOutput(lastDebugTime);

			// Check if animation is complete
			if (state.animation.animationTime >= state.animation.kTotalAnimationTime)
			{
				state.animation.isAnimating = false;
				state.animation.currentPosition = state.animation.endPosition;
				state.animation.velocity = Config::kZeroVec3;
				state.animation.currentSpeed = 0.0f;

				std::print("Animation COMPLETE\n");
			}
		}

		// Update camera state
		state.camera.updateVectors();

		// ------------- Setup camera pipeline -------------
		// TODO: Modularize this later
		// Projection matrix
		Mat44f projection = make_perspective_projection(Config::Rendering::kFOV, aspectRatio, Config::Rendering::kNearPlane, Config::Rendering::kFarPlane);

		// View matrix
		Mat44f view = state.camera.getViewMatrix();
		Mat44f projView = projection * view;

		// Model matrices
		/* CUBE */
		// Calculate cube transform
		Mat44f model2world_cube;
		if (state.animation.isAnimating ||
			(state.animation.animationTime >= state.animation.kTotalAnimationTime &&
			 state.animation.animationTime > 0.0f))
		{
			Mat44f rotation = calculate_rocket_rotation(state.animation);

			// Combine translation and rotation
			model2world_cube = make_translation(state.animation.currentPosition) *
							   make_scaling(0.5f, 0.5f, 0.5f) *
							   rotation;
		}
		else
		{
			// Static position at start (pre-launch)
			model2world_cube = make_translation(state.animation.startPosition) *
							   make_scaling(0.5f, 0.5f, 0.5f) *
							   make_rotation_y(angle * 0.3f);
		}

		Mat44f projCameraWorld_cube = make_proj_camera_world(projView, model2world_cube);
		Mat33f normalMatrix_cube = make_uniform_normal(model2world_cube);

		// TODO: Draw scene
		OGL_CHECKPOINT_DEBUG();

		// Clear every frame.
		// /*
		beginFrame();
		glUseProgram(unifiedProg.programId());

		// ------ Set lighting uniforms (shared by all objects) ------
		setLightingUniforms(
			Config::Rendering::kLightDir,
			Config::Rendering::kLightDiffuse,
			Config::Rendering::kSceneAmbient
		);

		// ----- Render Terrain -----
		drawTerrain(
			parlahtiVao,
			parlahtiVertexCount,
			parlahtiTexture,
			projView,
			kIdentity33f
		);

		// ----- Render Landing Pads (INSTANCED DRAWING) -----
		drawLandingPads(landingPads, projView);

		// ----- Render Cube -----
		drawColoredObject(
			cubeVao,
			cubeVertexCount,
			cubeMesh.indexCount(),
			projCameraWorld_cube,
			normalMatrix_cube
		);

		endFrame();
		// */

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers(window);
	}

	// Cleanup.
	state.prog = nullptr;

	// TODO: additional cleanup
	glDeleteVertexArrays(1, &parlahtiVao);
	glDeleteVertexArrays(1, &cubeVao);

	if (parlahtiTexture != 0)
		glDeleteTextures(1, &parlahtiTexture);

	LandingPad::cleanup();

	return 0;
}
catch (std::exception const &eErr)
{
	std::print(stderr, "Top-level Exception ({}):\n", typeid(eErr).name());
	std::print(stderr, "{}\n", eErr.what());
	std::print(stderr, "Bye.\n");
	return 1;
}

namespace
{
	void glfw_callback_error_(int aErrNum, char const *aErrDesc)
	{
		std::print(stderr, "GLFW error: {} ({})\n", aErrDesc, aErrNum);
	}

	void glfw_callback_key_(GLFWwindow *aWindow, int aKey, int, int aAction, int)
	{
		auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow));
		if (!state) return;

		// Handle key events
		// If action is PRESS or REPEAT (E.g., the key is held down), set movement flag to true; else false
		// The REPEAT action allows for multi-key presses to be recognised.
		// So, moving forwards and left at the same time is possible
		bool isPressed = (aAction == GLFW_PRESS || aAction == GLFW_REPEAT);

		switch (aKey)
		{
			case GLFW_KEY_ESCAPE:
				if (aAction == GLFW_PRESS)
				{
					// TODO: Remove later. For debugging.
					std::println("Exiting...");
					glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
				}
				break;

			case GLFW_KEY_R:
				if (aAction == GLFW_PRESS && state->prog)
				{
					try
					{
						state->prog->reload();
						std::print(stderr, "Shaders reloaded and recompiled.\n");
					}
					catch (std::exception const &eErr)
					{
						std::print(stderr, "Error when reloading shader:\n");
						std::print(stderr, "{}\n", eErr.what());
						std::print(stderr, "Keeping old shader.\n");
					}

					// Reset animation
					state->animation.reset();

					// Reset camera to Free mode but do not change it's initial position
					state->camera.setMode(Camera::Mode::Free);
					state->camera.updateVectors();

					std::print("Animation RESET - Vehicle returned to launch pad\n");
					std::print("Camera mode RESET to FREE\n");
				}
				break;

			case GLFW_KEY_W: state->input.moveForward = isPressed; break;
			case GLFW_KEY_S: state->input.moveBackward = isPressed; break;
			case GLFW_KEY_A: state->input.moveLeft = isPressed; break;
			case GLFW_KEY_D: state->input.moveRight = isPressed; break;
			case GLFW_KEY_E: state->input.moveUp = isPressed; break;
			case GLFW_KEY_Q: state->input.moveDown = isPressed; break;

			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_RIGHT_SHIFT:
				state->input.shiftPressed = isPressed;
				state->camera.setSpeed(isPressed ? Config::Camera::kBaseSpeed * Config::Camera::kSpeedFastMultiplier : Config::Camera::kBaseSpeed);
				break;

			case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_RIGHT_CONTROL:
				state->input.controlPressed = isPressed;
				state->camera.setSpeed(isPressed ? Config::Camera::kBaseSpeed * Config::Camera::kSpeedSlowMultiplier : Config::Camera::kBaseSpeed);
				break;

			// TODO: Remove later. For debugging.
			case GLFW_KEY_P:
				if (aAction == GLFW_PRESS)
				{
					// Position
					Vec3f pos = state->camera.getPosition();
					state->animation.printCoordinates(pos);

					// Orientation
					float yaw = state->camera.getYaw();
					float pitch = state->camera.getPitch();
					std::print("Camera yaw: {:2f} radians, pitch: {:2f} radians\n", yaw, pitch);
				}
				break;

			case GLFW_KEY_C:
				if (aAction == GLFW_PRESS)
				{
					state->camera.cycleMode();
					auto newMode = state->camera.getMode();

					switch (newMode)
					{
						using enum Camera::Mode;
						case Follow:
							state->camera.initFollowMode(
									state->animation.currentPosition,
									state->animation.velocity);
							break;

						case Fixed:
							state->camera.initFixedMode(
								state->animation.currentPosition);
							break;

						case Free:
						default:
							break;
					}
				}
				break;

			case GLFW_KEY_F:
				if (aAction == GLFW_PRESS)
				{
					if (!state->animation.isAnimating)
					{
						state->animation.start();
					}
					else
					{
						state->animation.togglePause();
					}
				}
				break;

			// TODO: Remove later. For debugging.
			case GLFW_KEY_G: // Faces vehicle from current position and orientation
				if (aAction == GLFW_PRESS)
				{
					/* TESTING FOLLOW CAMERA */
					// Calculate offset position to cube.
					// update position to be in-line with the cube but at a fixed distance
					Vec3f toCube = state->animation.currentPosition - state->camera.getPosition();

					// Follow distance should be a fixed distance of vec3f length
					toCube = normalize(toCube) * length(Vec3f{20.f, 5.f, 0.f});
					Vec3f newCamPos = state->animation.currentPosition - toCube;
					state->camera.setPosition(newCamPos);

					/* TESTING FIXED GROUND CAMERA
					// state->camera.setPosition(Vec3f{-37.45f, 17.90f, -48.04f});
					// 	// First, debug current state
					// state->camera.debugOrientation(state->animation.currentPosition);

					// // Then look at target
					// state->camera.lookAtTarget(state->animation.currentPosition);

					// // Debug after adjustment
					// std::print("\nAfter lookAtTarget:\n");
					// state->camera.debugOrientation(state->animation.currentPosition);

					// testCameraOrientation(state->camera, state->animation.currentPosition);
					*/
				}

				break;

			case GLFW_KEY_J: // TODO: Remove later. For debugging.
				if (aAction == GLFW_PRESS)
				{
					// Jump between the two landing pads
					static size_t currentPadIndex = 1;
					Vec2f orientation{-2.43f, -0.088f};
					Vec3f landingPadPos = (currentPadIndex == 1) ? Config::World::kLandingPad1Pos : Config::World::kLandingPad2Pos;

					state->camera.setPosition(landingPadPos + Vec3f{0.f, 0.5f, 0.f});
					state->camera.setYaw(orientation.x);
					state->camera.setPitch(orientation.y);

					std::print("Jumped to Landing Pad {} at position: ({}, {}, {})\n",
							   currentPadIndex,
							   landingPadPos.x,
							   landingPadPos.y,
							   landingPadPos.z);

					currentPadIndex = (currentPadIndex + 1) % 2; // Only 2 pads
				}
				break;

			default:
				break;
		}
	}

	void glfw_callback_mouse_button_(GLFWwindow *aWindow, int aButton, int aAction, int)
	{
		auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow));
		if (!state) return;

		if (aButton == GLFW_MOUSE_BUTTON_RIGHT && aAction == GLFW_PRESS)
		{
			if (!state->input.mouseLookActive)
			{
				// Activating mouse look
				double mouseX, mouseY;
				glfwGetCursorPos(aWindow, &mouseX, &mouseY);

				state->input.activateMouseLook(static_cast<float>(mouseX),
											   static_cast<float>(mouseY));

				// Hide cursor and capture it
				glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				std::print("Mouse look ENABLED\n");
			}
			else
			{
				// Deactivating mouse look and showing cursor
				state->input.deactivateMouseLook();
				glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				std::print("Mouse look DISABLED\n");
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow *aWindow, double aX, double aY)
	{
		auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow));
		if (!state || !state->input.mouseLookActive) return;

		// Handle mouse movement for camera orientation
		if (state->input.firstMouse)
		{
			// Use initial mouse position and don't apply rotation on this first frame
			state->input.updateMousePosition(static_cast<float>(aX), static_cast<float>(aY));
			state->input.firstMouse = false;
			return;
		}

		// Calculate the mouse's offset since the last frame.
		float xOffset = static_cast<float>(aX) - state->input.lastMouseX;
		float yOffset = state->input.lastMouseY - static_cast<float>(aY); // Reversed

		// Update last positions
		state->input.updateMousePosition(static_cast<float>(aX), static_cast<float>(aY));

		// Apply sensitivity
		xOffset *= Config::Camera::kSensitivity;
		yOffset *= Config::Camera::kSensitivity;

		// Rotate camera if there's significant movement (avoid micro-jitter)
		if (std::abs(xOffset) > Config::Camera::kDeadZone || std::abs(yOffset) > Config::Camera::kDeadZone)
		{
			state->camera.rotate(xOffset, yOffset);
		}
	}

}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if (window)
			glfwDestroyWindow(window);
	}
}
