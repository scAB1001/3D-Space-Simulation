#define CW2_ENABLE_GPU_TIMING
#ifdef CW2_ENABLE_GPU_TIMING
#include "gpu_timing.hpp"  // Use GPU timing utilities by defining this macro.
#endif

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

#include "defaults.hpp"
#include "config.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "state.hpp"
#include "animation.hpp"
#include "renderer.hpp"

#include "simple_mesh.hpp"
#include "space_vehicle.hpp"

#include "landing_pad.hpp"
#include "texture.hpp"
#include "loadobj.hpp"
#include "ui_text.hpp"
#include "ui_button.hpp"


namespace
{
	constexpr char const *kWindowTitle = "COMP3811 - CW2";

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
		GLFWwindow *window;
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

	#ifdef CW2_ENABLE_GPU_TIMING
	GpuTiming gpuTiming{};
	initGpuTiming(gpuTiming);

	double gpuSumMs = 0.0;
	double cpuSumMs = 0.0;
	int gpuFrameCount = 0;
	int cpuFrameCount = 0;
	#endif

	UITextRenderer uiText;
	uiText.init();
	UIButton launchButton;
	UIButton resetButton;

	std::print("RENDERER {}\n", (char const *)glGetString(GL_RENDERER));
	std::print("VENDOR {}\n", (char const *)glGetString(GL_VENDOR));
	std::print("VERSION {}\n", (char const *)glGetString(GL_VERSION));
	std::print("SHADING_LANGUAGE_VERSION {}\n", (char const *)glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Ddebug output
	#if !defined(NDEBUG) && !defined(CW2_ENABLE_GPU_TIMING)
	setup_gl_debug_output();
	#endif

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();
	globalGLSetup();
	OGL_CHECKPOINT_ALWAYS();

	int iwidth, iheight;
	glfwGetFramebufferSize(window, &iwidth, &iheight);
	glViewport(0, 0, iwidth, iheight);

	// Load global and particle shaders
	ShaderProgram unifiedProg({
		{ GL_VERTEX_SHADER, "assets/cw2/shaders/unified.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/shaders/unified.frag" }
	});
	state.prog = &unifiedProg;

	// Initialize particle system
	ShaderProgram particleProg({
		{ GL_VERTEX_SHADER,   "assets/cw2/shaders/particle.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/shaders/particle.frag" }
	});
	state.particles.setShader(particleProg.programId());
	state.particles.init();

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;

	// Create VBOs and VAO
	auto parlahtiMesh = load_wavefront_obj("assets/cw2/parlahti.obj");
	GLuint parlahtiVao = create_vao(parlahtiMesh);
	std::size_t parlahtiVertexCount = parlahtiMesh.vertexCount();
	GLuint parlahtiTexture = 0;
	if (parlahtiMesh.materialType == 1 && parlahtiMesh.hasTexcoords())
	{
		parlahtiTexture = load_texture_2d("assets/cw2/L4343A-4k.jpeg");
	}

	std::vector<LandingPad> landingPads = {
		LandingPad(Config::World::kLandingPad1Pos, Config::World::kLandingPadScale),
		LandingPad(Config::World::kLandingPad2Pos, Config::World::kLandingPadScale)};

	// Space vehicle mesh
	auto vehicleMesh = make_space_vehicle();
	vehicleMesh.materialType = 0; // coloured, no texture

	GLuint vehicleVao = create_vao(vehicleMesh);
	std::size_t vehicleVertexCount = vehicleMesh.vertexCount();
	std::size_t vehicleIndexCount = vehicleMesh.indexCount();

	// Reset state.
	resetBindings();

	// Prepare render context for main loop
	RenderContext renderCtx = {
		.parlahtiVao = parlahtiVao,
		.parlahtiVertexCount = parlahtiVertexCount,
		.parlahtiTexture = parlahtiTexture,
		.landingPads = landingPads,
		.vehicleVao = vehicleVao,
		.vehicleVertexCount = vehicleVertexCount,
		.vehicleIndexCount = vehicleIndexCount,
		.unifiedProg = unifiedProg.programId()};

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		#ifdef CW2_ENABLE_GPU_TIMING
		double cpuMs = 0.0;
		#endif

		// Let GLFW process events
		glfwPollEvents();

		#ifdef CW2_ENABLE_GPU_TIMING
		if (gpuTiming.frameIndex > GPU_QUERY_BUFFER_SIZE)
		{
			int prev = (gpuTiming.frameIndex - GPU_QUERY_BUFFER_SIZE)
					% GPU_QUERY_BUFFER_SIZE;

			GLint available = 0;
			glGetQueryObjectiv(
				gpuTiming.fullEnd[prev],
				GL_QUERY_RESULT_AVAILABLE,
				&available
			);

			if (available)
			{
				GLuint64 t0, t1;
				glGetQueryObjectui64v(
					gpuTiming.fullStart[prev], GL_QUERY_RESULT, &t0);
				glGetQueryObjectui64v(
					gpuTiming.fullEnd[prev], GL_QUERY_RESULT, &t1);

				gpuSumMs += (t1 - t0) * 1e-6;
				gpuFrameCount++;
			}
		}
		#endif

		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize(window, &nwidth, &nheight);

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if (0 == nwidth || 0 == nheight)
			{
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize(window, &nwidth, &nheight);
				} while (0 == nwidth || 0 == nheight);
			}

			glViewport(0, 0, nwidth, nheight);
		}

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;

		angle += dt * Config::kFloatPi * 0.3f;
		if (angle >= 2.f * Config::kFloatPi)
			angle -= 2.f * Config::kFloatPi;

		// Update camera state
		state.camera.updateVectors();
		if (state.input.mouseLookActive)
		{
			if (!state.splitScreenEnabled)
			{
				// Single-view mode: move main camera only
				processMovement(state.camera, state.input, dt);
			}
			else
			{
				// Split-screen mode: move BOTH cameras
				processMovement(state.leftCamera, state.input, dt);
				processMovement(state.rightCamera, state.input, dt);
			}
		}

		// Update animation state
		if (state.animation.isAnimating && !state.animation.isPaused)
		{
			Vec3f enginePos = state.animation.currentPosition - Vec3f{0.f, 0.3f, 0.f};
			Vec3f exhaustDir = Vec3f{0.f, -1.f, 0.f};

			for (int i = 0; i < 6; i++)
				state.particles.emit(enginePos, exhaustDir);
			state.animation.update(dt);

			// Update animation cameras
			state.camera.updateForAnimation(
				state.animation.currentPosition,
				state.animation.velocity, dt);

			if (state.splitScreenEnabled)
			{
				state.leftCamera.updateForAnimation(
					state.animation.currentPosition,
					state.animation.velocity, dt);

				state.rightCamera.updateForAnimation(
					state.animation.currentPosition,
					state.animation.velocity, dt);
			}

			// Check if animation is complete
			if (state.animation.animationTime >= state.animation.kTotalAnimationTime)
			{
				state.animation.isAnimating = false;
				state.animation.currentPosition = state.animation.endPosition;
				state.animation.velocity = Config::kZeroVec3;
			}
		}
		state.particles.update(dt);

		// Update camera state
		state.camera.updateVectors();

		// Model matrices
		Mat44f model2world_vehicle;
		if (state.animation.isAnimating ||
			(state.animation.animationTime >= state.animation.kTotalAnimationTime &&
			 state.animation.animationTime > 0.0f))
		{
			Mat44f rotation = calculate_rocket_rotation(state.animation);
			model2world_vehicle =
				make_translation(state.animation.currentPosition) *
				make_scaling(0.5f, 0.5f, 0.5f) * rotation;
		}
		else
		{
			model2world_vehicle =
				make_translation(state.animation.startPosition) *
				make_scaling(0.5f, 0.5f, 0.5f) *
				make_rotation_y(angle * 0.3f);
		}

		OGL_CHECKPOINT_DEBUG();
		#ifdef CW2_ENABLE_GPU_TIMING
		int q = gpuTiming.frameIndex % GPU_QUERY_BUFFER_SIZE;
		glQueryCounter(gpuTiming.fullStart[q], GL_TIMESTAMP);
		auto cpuStart = std::chrono::high_resolution_clock::now();

		#endif

		beginFrame();

		renderSingleOrSplitScreen(
			state,
			model2world_vehicle,
			fbwidth,
			fbheight,
			state.particles,
			renderCtx);

		OGL_CHECKPOINT_DEBUG();

		// endFrame();
		#ifdef CW2_ENABLE_GPU_TIMING
		auto cpuEnd = std::chrono::high_resolution_clock::now();
		cpuMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();
		cpuSumMs += cpuMs;
		cpuFrameCount++;
		#endif

		#ifdef CW2_ENABLE_GPU_TIMING
		glQueryCounter(gpuTiming.fullEnd[q], GL_TIMESTAMP);
		gpuTiming.frameIndex++;
		#endif

		// Reset Viewport for UI
		glViewport(0, 0, fbwidth, fbheight);

		// Current screen dimensions
		int screenW = static_cast<int>(fbwidth);
		int screenH = static_cast<int>(fbheight);

		// Render Altitude
		uiText.renderAltitude(
			static_cast<int>(state.animation.currentPosition.y),
			screenW,
			screenH);

		// Launch and Reset button positioning
		float totalBtnWidth = UITextRenderer::btnW * 2.f + UITextRenderer::spacing;
		float startBtnX = (screenW - totalBtnWidth) * 0.5f;
		Vec2f btnPos = {startBtnX, UITextRenderer::marginBtm};
		Vec2f btnSize = {UITextRenderer::btnW, UITextRenderer::btnH};

		// Mouse and cursor state over button
		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		float mouseX = static_cast<float>(mx);
		float mouseY = screenH - static_cast<float>(my);

		updateButton(launchButton, btnPos, btnSize, "LAUNCH", mouseX, mouseY, mouseDown);
		getNextButtonPos(launchButton, btnPos, UITextRenderer::spacing);
		updateButton(resetButton,  btnPos, btnSize, "RESET", mouseX, mouseY, mouseDown);

		// Draw latest button state on-screen
		drawButton(launchButton, uiText, screenW, screenH);
		drawButton(resetButton,  uiText, screenW, screenH);

		// Actions
		if (launchButton.clicked && !state.animation.isAnimating)
			state.animation.start();

		if (resetButton.clicked)
			glfw_callback_key_(window, GLFW_KEY_R, 0, GLFW_PRESS, 0);

		endFrame();
		OGL_CHECKPOINT_DEBUG();

		glfwSwapBuffers(window);
	}

	cleanup(state, parlahtiVao, vehicleVao, parlahtiTexture, 0, uiText);

	#ifdef CW2_ENABLE_GPU_TIMING
	if (gpuFrameCount > 0)
	{
		std::print("Average GPU frame time: {:.3f} ms\n",
				gpuSumMs / gpuFrameCount);
	}

	if (cpuFrameCount > 0)
	{
		std::print("Average CPU frame time: {:.3f} ms\n",
				cpuSumMs / cpuFrameCount);
	}

	destroyGpuTiming(gpuTiming);
	#endif

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
		if (!state)
			return;

		bool isPressed = (aAction == GLFW_PRESS || aAction == GLFW_REPEAT);
		switch (aKey)
		{
			case GLFW_KEY_ESCAPE:
				if (aAction == GLFW_PRESS)
				{
					glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
				}
				break;

			case GLFW_KEY_R:
				if (aAction == GLFW_PRESS && state->prog)
				{
					// Reset animation
					state->animation.reset();

					// Reset camera to Free mode but do not change it's initial position
					state->camera.setMode(Camera::Mode::Free);
					state->camera.updateVectors();
				}
				break;
			case GLFW_KEY_1:
				if (aAction == GLFW_PRESS) state->pointLights[0].toggle();
				break;

			case GLFW_KEY_2:
				if (aAction == GLFW_PRESS) state->pointLights[1].toggle();
				break;

			case GLFW_KEY_3:
				if (aAction == GLFW_PRESS) state->pointLights[2].toggle();
				break;

			case GLFW_KEY_4:
				if (aAction == GLFW_PRESS) state->toggleGlobalDirLight();
				break;

			case GLFW_KEY_W:
				state->input.moveForward = isPressed;
				break;
			case GLFW_KEY_S:
				state->input.moveBackward = isPressed;
				break;
			case GLFW_KEY_A:
				state->input.moveLeft = isPressed;
				break;
			case GLFW_KEY_D:
				state->input.moveRight = isPressed;
				break;
			case GLFW_KEY_E:
				state->input.moveUp = isPressed;
				break;
			case GLFW_KEY_Q:
				state->input.moveDown = isPressed;
				break;

			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_RIGHT_SHIFT:
				state->input.shiftPressed = isPressed;
				state->camera.updateSpeed(state->camera, state->input);
				state->camera.updateSpeed(state->leftCamera, state->input);
				state->camera.updateSpeed(state->rightCamera, state->input);
				break;

			case GLFW_KEY_LEFT_CONTROL:
			case GLFW_KEY_RIGHT_CONTROL:
				state->input.controlPressed = isPressed;
				state->camera.updateSpeed(state->camera, state->input);
				state->camera.updateSpeed(state->leftCamera, state->input);
				state->camera.updateSpeed(state->rightCamera, state->input);
				break;

			case GLFW_KEY_C:
				if (aAction == GLFW_PRESS)
				{
					if (!state->splitScreenEnabled)
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

							default:
								break;
						}
					}
					else
					{
						if (state->input.shiftPressed)
						{
							// RIGHT CAMERA
							state->rightCamera.cycleMode();
							auto newMode = state->rightCamera.getMode();

							switch (newMode)
							{
								using enum Camera::Mode;
								case Follow:
									state->rightCamera.initFollowMode(
										state->animation.currentPosition,
										state->animation.velocity);
									break;

								case Fixed:
									state->rightCamera.initFixedMode(
										state->animation.currentPosition);
									break;

								default:
									break;
							}
						}
						else
						{
							// LEFT CAMERA
							state->leftCamera.cycleMode();
							auto newMode = state->leftCamera.getMode();

							switch (newMode)
							{
								using enum Camera::Mode;
								case Follow:
									state->leftCamera.initFollowMode(
										state->animation.currentPosition,
										state->animation.velocity);
									break;

								case Fixed:
									state->leftCamera.initFixedMode(
										state->animation.currentPosition);
									break;

								default:
									break;
							}
						}
					}
				}
				break;

			case GLFW_KEY_V:
				if (aAction == GLFW_PRESS)
				{
					state->toggleSplitScreen();
					if (state->splitScreenEnabled)
					{
						// Setup cameras for split screen
						// Left camera = same as main camera
						state->leftCamera = state->camera;

						// Right camera = FOLLOW mode
						state->rightCamera = state->camera;
						state->rightCamera.setMode(Camera::Mode::Follow);
						state->rightCamera.initFollowMode(
							state->animation.currentPosition,
							state->animation.velocity
						);
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

			default:
				break;
		}
	}

	void glfw_callback_mouse_button_(GLFWwindow *aWindow, int aButton, int aAction, int)
	{
		auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow));
		if (!state)
			return;

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
			}
			else
			{
				// Deactivating mouse look and showing cursor
				state->input.deactivateMouseLook();
				glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow *aWindow, double aX, double aY)
	{
		auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow));
		if (!state || !state->input.mouseLookActive)
			return;

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

		// Rotate camera if there's significant movement (avoids micro-jitter)
		if (std::abs(xOffset) > Config::Camera::kDeadZone || std::abs(yOffset) > Config::Camera::kDeadZone)
		{
			if (!state->splitScreenEnabled)
			{
				// Single-view: rotate only main camera
				state->camera.rotate(xOffset, yOffset);
			}
			else
			{
				// Split-screen: rotate BOTH cameras
				state->leftCamera.rotate(xOffset, yOffset);
				state->rightCamera.rotate(xOffset, yOffset);
			}
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