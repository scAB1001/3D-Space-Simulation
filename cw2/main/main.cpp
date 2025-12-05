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
#include "cone.hpp"
#include "cylinder.hpp"
#include "simple_mesh.hpp"
#include "texture.hpp"
#include "loadobj.hpp"
#include "landing_pad.hpp"
#include "renderer.hpp"
#include "test.hpp"

// TODO: LIST
// - 1.4 CALL DRAW_ARRAY TWICE
// - 1.6 BILL-PHONG GOES IN FRAG ONLY
// - 1.7 USE AN EQUATION THAT IS A FUNCTION T, AS T INCREASES, SLOW ACCELERATE FROM STANDSTILL
// - 1.10 DRAW AS MANY AS POSSIBLE WITHOUT LOSING FPS (60 MAX for ~1,000,000), VARIABLE (LIKE T particles)

namespace
{
	constexpr char const *kWindowTitle = "COMP3811 - CW2";

	// Camera struct to hold camera state and methods
	struct Camera {
		/* Update camera state
		 * Radius controls vertical distance from cameraTarget point (here: origin)
		 * Phi controls horizontal angle around the cameraTarget point
		 * Theta controls vertical angle from the horizontal plane
		 */

		// CITE: https://learnopengl.com/Getting-started/Camera
		Vec3f position = Config::kInitialCameraPos;
        Vec3f forward = {0.f, 0.f, -1.f};
		Vec3f right = {1.f, 0.f, 0.f};
		Vec3f worldUp = {0.f, 1.f, 0.f};
		Vec3f up = {0.f, 1.f, 0.f};

		float yaw = Config::kInitialCameraYaw; // Left/right rotation (around Y)
		float pitch = 0.f;					   // Up/down rotation (around X)
		float speed = Config::kCameraBaseSpeed;

		/* Note: that we normalize the resulting right vector.
		 * If we wouldn't normalize this vector, the resulting cross product may return
		 *	differently sized vectors based on the cameraFront variable.

		 * If we would not normalize the vector we would move slow or fast
		 *	based on the camera's orientation instead of at a consistent movement speed.
		 */
		void updateVectors() {
            forward.x = std::cos(yaw) * std::cos(pitch);
            forward.y = std::sin(pitch);
            forward.z = std::sin(yaw) * std::cos(pitch);
            forward = normalize(forward);

			// Recalculate right and up vectors
            right = normalize(cross(forward, worldUp));

			// Calculate actual up vector (perpendicular to forward and right)
            up = normalize(cross(right, forward));
        }

        Mat44f getViewMatrix() const {
            return make_look_at(position, position + forward, up);
        }
    };

    struct InputState {
        bool moveForward = false;
        bool moveBackward = false;
        bool moveLeft = false;
        bool moveRight = false;
        bool moveUp = false;
        bool moveDown = false;
        bool mouseLookActive = false;
		bool cameraActive = true; // TODO: Remove later. For debugging.
        float lastMouseX = 0.f;
        float lastMouseY = 0.f;
        bool firstMouse = true;
    };

	struct AnimationState
	{
		bool isAnimating = false;
		bool isPaused = false;
		float animationTime = 0.0f; // T in the equation
		float totalAnimationTime = 0.0f;
		Vec3f startPosition = {0.f, 2.f, 0.f};	 // Cube's current position
		Vec3f initialPosition = {0.f, 2.f, 0.f}; // For reset
		Mat44f initialTransform = kIdentity44f;

		// Animation parameters
		float maxHeight = 20.0f;		  // Peak height
		float horizontalDistance = 50.0f; // How far it travels
		float animationDuration = 10.0f;  // Total time in seconds
		float acceleration = 2.0f;		  // Acceleration factor
	};

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

	// Load main shader program
	ShaderProgram unifiedProg( {
		{ GL_VERTEX_SHADER, "assets/cw2/shaders/unified.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/shaders/unified.frag" }
	} );
	state.prog = &unifiedProg;

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;

	//TODO: create VBOs and VAO
	// 0. Load parlahti
	auto parlahtiMesh = load_wavefront_obj("assets/cw2/parlahti.obj");
	GLuint parlahtiVao = create_vao(parlahtiMesh);
	std::size_t parlahtiVertexCount = parlahtiMesh.vertexCount();
	std::println("Terrain loaded: {} vertices, material type: {}",
                 parlahtiVertexCount, parlahtiMesh.materialType);

	// Load parlahti texture if available
	GLuint parlahtiTexture = 0;
	if (parlahtiMesh.materialType == 1 && parlahtiMesh.hasTexcoords())
	{
		std::println( "Loading parlahti texture..." );
		parlahtiTexture = load_texture_2d("assets/cw2/L4343A-4k.jpeg");
	}

	// Create landing pads (instances)
	std::vector<LandingPad> landingPads = {
		LandingPad(Config::kLandingPad1Pos, Config::kLandingPadScale),
		LandingPad(Config::kLandingPad2Pos, Config::kLandingPadScale)
	};

	/* CUBE */
	// auto cubeMesh = make_cube_with_normals({0.8f, 0.2f, 0.2f});
	auto cubeMesh = make_indexed_cube({0.8f, 0.2f, 0.2f});
	cubeMesh.materialType = 0; // Colored
	GLuint cubeVao = create_vao(cubeMesh);
	std::size_t cubeVertexCount = cubeMesh.vertexCount();

	/* ARROWS */
	auto baseCylinder = make_batched_indexed_cylinder(true, 16, {0.2f, 0.8f, 0.2f});
	auto baseCone = make_batched_indexed_cone(true, 16, {0.8f, 0.2f, 0.2f}, make_translation(Vec3f{1.f, 0.f, 0.f}) * make_scaling(0.5f, 0.5f, 0.5f));

	// 1. Test concatenate()
	auto testArrowMesh_concat = concatenate(baseCylinder, baseCone);
	testArrowMesh_concat.materialType = 0; // Colored
	GLuint testArrowVao_concat = create_vao(testArrowMesh_concat);
	std::size_t testArrowVertexCount_concat = testArrowMesh_concat.vertexCount();

	// 2. Test concatenate_many()
	auto testArrowMesh_concat_many = concatenate_many({baseCylinder, baseCone});
	testArrowMesh_concat_many.materialType = 0; // Colored
	GLuint testArrowVao_concat_many = create_vao(testArrowMesh_concat_many);
	std::size_t testArrowVertexCount_concat_many = testArrowMesh_concat_many.vertexCount();

	// 3. Test create_vao_from_meshes()
	GLuint testArrowVao_from_meshes = create_vao_from_meshes({baseCylinder, baseCone});
	std::size_t testArrowVertexCount_from_meshes = baseCylinder.vertexCount() + baseCone.vertexCount();

	// -------------- Run tests --------------
	// test_all_mesh_functions();
	// test_vao_creation();

	// Reset state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

		// Handle camera movement
		// TODO: Remove cameraActive later. For debugging.
		if (state.input.mouseLookActive && state.input.cameraActive) {
            float moveSpeed = state.camera.speed * dt;

            if (state.input.moveForward) state.camera.position += state.camera.forward * moveSpeed;
            if (state.input.moveBackward) state.camera.position -= state.camera.forward * moveSpeed;
            if (state.input.moveLeft) state.camera.position -= state.camera.right * moveSpeed;
            if (state.input.moveRight) state.camera.position += state.camera.right * moveSpeed;
            if (state.input.moveUp) state.camera.position += state.camera.up * moveSpeed;
            if (state.input.moveDown) state.camera.position -= state.camera.up * moveSpeed;

            // Clamp vertical movement only
            state.camera.position.y = std::clamp(state.camera.position.y, Config::kSeaLevel + 0.5f, 50.f);

			// Clamp position to within parlahti bounds
			state.camera.position.x = std::clamp(state.camera.position.x, -Config::kWorldBorder, Config::kWorldBorder);
			state.camera.position.z = std::clamp(state.camera.position.z, -Config::kWorldBorder, Config::kWorldBorder);
        }

		if (state.animation.isAnimating && !state.animation.isPaused)
		{
			state.animation.animationTime += dt;
			state.animation.totalAnimationTime += dt;

			// Check if animation is complete
			if (state.animation.animationTime >= state.animation.animationDuration)
			{
				state.animation.isAnimating = false;
				std::print("Animation COMPLETED\n");
			}
		}

		// ------------- Setup camera pipeline -------------
		// TODO: Modularise this later
		// Projection matrix
		Mat44f projection = make_perspective_projection(Config::kFOV, aspectRatio, Config::kNearPlane, Config::kFarPlane);

		// View matrix
		Mat44f view = state.camera.getViewMatrix();
		Mat44f projView = projection * view;

		// Model matrices
		/* TEST ARROWS*/
		// 1. Test concatenate()
		Mat44f model2world_testArrow_concat = make_rotation_y(angle) * make_translation(Vec3f{-12.f, 0.f, 0.f});
		Mat44f projCameraWorld_testArrow_concat = make_proj_camera_world(projView, model2world_testArrow_concat);
		Mat33f normalMatrix_testArrow_concat = make_uniform_normal(model2world_testArrow_concat);

		// 2. Test concatenate_many()
		Mat44f model2world_arrow_concat_many = make_translation(Vec3f{-8.f, 0.f, 0.f});
		Mat44f projCameraWorld_arrow_concat_many = make_proj_camera_world(projView, model2world_arrow_concat_many);
		Mat33f normalMatrix_arrow_concat_many = make_uniform_normal(model2world_arrow_concat_many);

		// 3. Test create_vao_from_meshes()
		Mat44f model2world_arrow_from_meshes = make_rotation_y(-angle) * make_translation(Vec3f{-4.f, 0.f, 0.f});
		Mat44f projCameraWorld_arrow_from_meshes = make_proj_camera_world(projView, model2world_arrow_from_meshes);
		Mat33f normalMatrix_arrow_from_meshes = make_uniform_normal(model2world_arrow_from_meshes);

		/* CUBE */
		// Calculate cube position (space vehicle)
		Mat44f cubeTransform;
		if (state.animation.isAnimating)
		{
			// Animated position
			Vec3f cubePos = calculateSpaceVehiclePosition(
				state.animation.animationTime,
				state.animation.animationDuration,
				state.animation.startPosition,
				state.animation.maxHeight,
				state.animation.horizontalDistance,
				state.animation.acceleration);

			// Calculate velocity for rotation (simplified)
			Vec3f velocity = cubePos - state.animation.startPosition;

			// Get rotation
			Mat44f rotation = calculateSpaceVehicleRotation(
				state.animation.animationTime,
				state.animation.animationDuration,
				velocity);

			// Combine translation and rotation
			cubeTransform = make_translation(cubePos) * rotation;

			// Update camera to follow vehicle (optional)
			if (state.animation.animationTime > 0.5f) // After initial delay
			{
				// Smooth camera follow
				Vec3f cameraOffset = {0.f, 5.f, -10.f};	  // Behind and above
				Mat44f invRotation = transpose(rotation); // Inverse rotation for camera offset

				// Transform camera offset by vehicle rotation
				Vec4f transformedOffset = invRotation * Vec4f{cameraOffset.x, cameraOffset.y, cameraOffset.z, 0.f};

				// Update camera position to follow vehicle
				state.camera.position = cubePos + Vec3f{transformedOffset.x, transformedOffset.y, transformedOffset.z};

				// Make camera look at vehicle
				state.camera.forward = normalize(cubePos - state.camera.position);
				state.camera.updateVectors();
			}
		}
		else
		{
			// Static position
			cubeTransform = make_rotation_y(angle) * make_translation(state.animation.initialPosition);
		}

		// Mat44f model2world_cube = make_rotation_y(angle) * make_translation(Vec3f{0.f, 2.f, 0.f});
		// Mat44f projCameraWorld_cube = make_proj_camera_world(projView, model2world_cube);
		// Mat33f normalMatrix_cube = make_uniform_normal(model2world_cube);
		Mat44f projCameraWorld_cube = make_proj_camera_world(projView, cubeTransform);
		Mat33f normalMatrix_cube = make_uniform_normal(cubeTransform);

		// TODO: Draw scene
		OGL_CHECKPOINT_DEBUG();

		// Clear every frame.
		beginFrame();
		glUseProgram(unifiedProg.programId());

		// ------ Set lighting uniforms (shared by all objects) ------
		setLightingUniforms(
			Config::kLightDir,
			Config::kLightDiffuse,
			Config::kSceneAmbient
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
		for (const auto &pad : landingPads)
		{
			Mat44f projCameraWorld_pad = projView * pad.transform;
			Mat33f normalMatrix_pad = make_uniform_normal(pad.transform);

			drawLandingPad(
				pad.vao,
				pad.vertexCount,
				projCameraWorld_pad,
				normalMatrix_pad
			);
		}

		// ----- Render Test Arrows -----
		drawColoredObject(
			testArrowVao_concat,
			testArrowVertexCount_concat,
			testArrowMesh_concat.indexCount(),
			projCameraWorld_testArrow_concat,
			normalMatrix_testArrow_concat
		);

		drawColoredObject(
			testArrowVao_concat_many,
			testArrowVertexCount_concat_many,
			testArrowMesh_concat_many.indexCount(),
			projCameraWorld_arrow_concat_many,
			normalMatrix_arrow_concat_many
		);

		drawColoredObject(
			testArrowVao_from_meshes,
			testArrowVertexCount_from_meshes,
			baseCylinder.indexCount() + baseCone.indexCount(),
			projCameraWorld_arrow_from_meshes,
			normalMatrix_arrow_from_meshes
		);

		// ----- Render Cube -----
		drawColoredObject(
			cubeVao,
			cubeVertexCount,
			cubeMesh.indexCount(),
			projCameraWorld_cube,
			normalMatrix_cube
		);

		// Cleanup the modified global state: Reset VAO and program.
		endFrame();

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers(window);
	}

	// Cleanup.
	state.prog = nullptr;

	// TODO: additional cleanup
	glDeleteVertexArrays(1, &testArrowVao_concat);
	glDeleteVertexArrays(1, &testArrowVao_concat_many);
	glDeleteVertexArrays(1, &testArrowVao_from_meshes);

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
					glfwSetWindowShouldClose(aWindow, GLFW_TRUE);
				break;

			case GLFW_KEY_R:
				if (aAction == GLFW_PRESS && state->prog)
					try
					{
						state->prog->reload();
						std::print(stderr, "Shaders reloaded and recompiled.\n");

						// Reset animation
						state->animation.isAnimating = false;
						state->animation.isPaused = false;
						state->animation.animationTime = 0.0f;
						state->animation.totalAnimationTime = 0.0f;
						std::print("Animation RESET\n");
					}
					catch (std::exception const &eErr)
					{
						std::print(stderr, "Error when reloading shader:\n");
						std::print(stderr, "{}\n", eErr.what());
						std::print(stderr, "Keeping old shader.\n");
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
				state->camera.speed = isPressed ? Config::kCameraBaseSpeed * 5.f : Config::kCameraBaseSpeed;
				// TODO: Remove later. For debugging.
				// std::print("Shift pressed\n");

				// if (GLFW_RELEASE == aAction)
				// 	std::print("Shift released\n");
				break;

			case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_RIGHT_CONTROL:
                state->camera.speed = isPressed ? Config::kCameraBaseSpeed * 0.2f : Config::kCameraBaseSpeed;

				// TODO: Remove later. For debugging.
				// std::print("Ctrl pressed\n");

				// if (GLFW_RELEASE == aAction)
				// 	std::print("Ctrl released\n");
				break;

			case GLFW_KEY_SPACE: // TODO: Remove later. For debugging.
				if (aAction == GLFW_PRESS)
				{
					state->input.cameraActive = !state->input.cameraActive;
					std::print("Camera movement: {}\n", state->input.cameraActive ? "ENABLED" : "DISABLED");
				}
				break;

			case GLFW_KEY_C: // TODO: Remove later. For debugging.
				if (aAction == GLFW_PRESS)
				{
					// Position
					std::print("Camera position: ({}, {}, {})\n",
							   state->camera.position.x,
							   state->camera.position.y,
							   state->camera.position.z);
					// Orientation
					std::print("Camera yaw: {} radians, pitch: {} radians\n",
							   state->camera.yaw,
							   state->camera.pitch);

					// Convert to degrees for easier understanding
					float yawDeg = state->camera.yaw * (180.f / Config::kFloatPi);
					float pitchDeg = state->camera.pitch * (180.f / Config::kFloatPi);

					// 2 decimal places
					yawDeg = std::round(yawDeg * 100.f) / 100.f;
					pitchDeg = std::round(pitchDeg * 100.f) / 100.f;
					std::print("Camera yaw: {} degrees, pitch: {} degrees\n",
							   yawDeg,
							   pitchDeg);
				}
				break;

			case GLFW_KEY_F:
				if (aAction == GLFW_PRESS)
				{
					if (!state->animation.isAnimating)
					{
						// Start animation
						state->animation.isAnimating = true;
						state->animation.isPaused = false;
						state->animation.animationTime = 0.0f;
						state->animation.startPosition = state->animation.initialPosition;
						std::print("Animation STARTED\n");
					}
					else
					{
						// Toggle pause
						state->animation.isPaused = !state->animation.isPaused;
						std::print("Animation {}PAUSED\n", state->animation.isPaused ? "" : "UN");
					}
				}
				break;

			case GLFW_KEY_G:
				if (aAction == GLFW_PRESS)
				{
					// Reset camera to initial position and orientation
					state->camera.position = Vec3f{-72.799965, -0.96877396, 10.692477};
					state->camera.yaw = 1.2900052f;
					state->camera.pitch = -0.06900009f;
				}
				break;

			case GLFW_KEY_J: // TODO: Remove later. For debugging.
				if (aAction == GLFW_PRESS)
				{
					// Jump between the two landing pads
					static size_t currentPadIndex = 1;
					Vec2f orientation{-2.43f, -0.088f};
					Vec3f landingPadPos = (currentPadIndex == 1) ? Config::kLandingPad1Pos : Config::kLandingPad2Pos;

					state->camera.position = landingPadPos + Vec3f{0.f, 0.5f, 0.f};
					state->camera.yaw = orientation.x;
					state->camera.pitch = orientation.y;

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
			// Toggle mouse look mode, hide cursor
			state->input.mouseLookActive = !state->input.mouseLookActive;
			glfwSetInputMode(aWindow, GLFW_CURSOR,
				state->input.mouseLookActive ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

			if (state->input.mouseLookActive)
			{
				state->input.firstMouse = true;
				// TODO: Remove later. For debugging.
				// std::print("Mouse Look: {}\n", state->input.mouseLookActive ? "ENABLED" : "DISABLED");
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
			state->input.lastMouseX = static_cast<float>(aX);
			state->input.lastMouseY = static_cast<float>(aY);
			state->input.firstMouse = false;
		}

		// Calculate the mouse's offset since the last frame.
		float xOffset = static_cast<float>(aX) - state->input.lastMouseX;
		float yOffset = state->input.lastMouseY - static_cast<float>(aY); // Reversed

		state->input.lastMouseX = static_cast<float>(aX);
		state->input.lastMouseY = static_cast<float>(aY);

		// Apply sensitivity
		xOffset *= Config::kCameraSensitivity;
		yOffset *= Config::kCameraSensitivity;

		// Update yaw and pitch
		state->camera.yaw += xOffset;
		state->camera.pitch += yOffset;

		// Constrain pitch to avoid gimbal lock ~89 degrees or pi/2.1 radians
		constexpr float maxPitch = Config::kFloatPi / 2.1f;
		state->camera.pitch = std::clamp(state->camera.pitch, -maxPitch, maxPitch);

		// Normalize yaw to the range [-pi, pi] for numerical stability
		if (state->camera.yaw > Config::kFloatPi)
			state->camera.yaw -= 2.f * Config::kFloatPi;
		if (state->camera.yaw < -Config::kFloatPi)
			state->camera.yaw += 2.f * Config::kFloatPi;
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
