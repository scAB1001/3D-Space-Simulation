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

// Added later
#include "cube.hpp" // TODO: Remove if not using cube data
#include "simple_mesh.hpp"
#include "loadobj.hpp"
#include "texture.hpp"

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";
	constexpr float kFloatPi = std::numbers::pi_v<float>;

	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			// Toggle camera control mode
			bool cameraActive = true; // TODO: Remove camera toggle
			// Toggle mouse look mode
			bool mouseLookActive = false;

			// Camera position and orientation
			Vec3f position = {0.f, 5.f, 10.f}; // Start position
			float yaw = -90.f;				   // Left/right rotation (around Y) (default: looking along -Z)
			float pitch = 0.f;				   // Up/down rotation (around X)

			// Movement flags
			bool moveForward = false;
			bool moveBackward = false;
			bool moveLeft = false;
			bool moveRight = false;
			bool moveUp = false;
			bool moveDown = false;

			// Speed control
			float mouseSensitivity = 0.001f; // TODO: May need adjusting
			float baseSpeed = 5.f;			 // units per second
			float currentSpeed = 5.f;

			// Mouse state
			float lastX = 0.f;
			float lastY = 0.f;
			bool firstMouse = true;
		} camControl;
	};

	void glfw_callback_error_( int, char const* );
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

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '{}' ({})", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	// Resizable window by default
	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	// Non-apple devices
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

	// Core profile
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	// 24 bits of depth buffer
	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '{}' ({})", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: Set up event handling_
	State_ state{};

	// Setup the additional GLFW callbacks
	glfwSetWindowUserPointer( window, &state );
	glfwSetKeyCallback( window, &glfw_callback_key_ );
	glfwSetMouseButtonCallback(window, &glfw_callback_mouse_button_);
	glfwSetCursorPosCallback(window, &glfw_callback_motion_);
	//////////////////////////////////////////////////////////////////////////////////

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoadGLLoader() failed - cannot load GL API!" );

	std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
	std::print( "VENDOR {}\n", (char const*)glGetString( GL_VENDOR ) );
	std::print( "VERSION {}\n", (char const*)glGetString( GL_VERSION ) );
	std::print( "SHADING_LANGUAGE_VERSION {}\n", (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: Global GL setup goes here
	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// Enable sRGB framebuffer to correct gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Enable face culling to avoid drawing back faces
	glEnable(GL_CULL_FACE);
	// This is the default, but set it explicitly
	glCullFace(GL_BACK);

	// Define front faces to be counter-clockwise
	glFrontFace(GL_CCW);

	// Enable depth testing for correct occlusion
	glEnable(GL_DEPTH_TEST);
	// This is the default, but set it explicitly
	glDepthFunc(GL_LESS);
	// Set clear depth to far plane (1.0) to match depth test
	// TODO: glad_glDepthRangef(0.f, 1.f);
	glClearDepthf(1.f);

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f); // dark gray - values between 0.0 and 1.0

	OGL_CHECKPOINT_ALWAYS();
	//////////////////////////////////////////////////////////////////////////////////

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// TODO: Setup shader program
	// Load shader program
	ShaderProgram unifiedProg( {
		{ GL_VERTEX_SHADER, "assets/cw2/shaders/unified.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/shaders/unified.frag" }
	} );

	/* Important Note: There's a trick here.
	 * We're storing texcoords as vec2 in the buffer,
	 * 	but the shader reads them as vec3 at location 1.
	 *
	 * This works because:
	 * 	Texcoords: vec2(x, y) in buffer -> read as vec3(x, y, ?) in shader
	 * 	The z-component is ignored for texcoords.
	 *
	 * For colors: vec3(r, g, b) in buffer -> read as vec3(r, g, b) in shader
	 */

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: global GL setup goes here
	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

	// Set up initial state
	state.prog = &unifiedProg;

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;
	//////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: Setup scene objects e.g. VAOs, VBOs, textures, uniforms, etc.
	/* PARLAHTI */
	auto parlahtiMesh = load_wavefront_obj("assets/cw2/parlahti.obj");
	parlahtiMesh.materialType = 1;                      // Textured material
	GLuint parlahtiVao = create_vao(parlahtiMesh);
	std::size_t parlahtiVertexCount = parlahtiMesh.vertexCount();
	GLuint parlahtiTexture = 0;
	if (parlahtiMesh.materialType == 1 && parlahtiMesh.hasTexcoords())
	{
		std::println("Loading terrain orthophoto for texture...");
		parlahtiTexture = load_texture_2d("assets/cw2/L4343A-4k.jpeg");
		// Update texture parameters to REPEAT
		// glBindTexture(GL_TEXTURE_2D, parlahtiTexture);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// glBindTexture(GL_TEXTURE_2D, 0);
	}

	/* CUBE */
	auto cubeMesh = make_cube_with_normals({0.8f, 0.2f, 0.2f});
	cubeMesh.materialType = 0; // Colored
	GLuint cubeVao = create_vao(cubeMesh);
	std::size_t cubeVertexCount = cubeMesh.vertexCount();

	// Reset state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	OGL_CHECKPOINT_ALWAYS();
	//////////////////////////////////////////////////////////////////////////////////

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();

		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		//////////////////////////////////////////////////////////////////////////////////
		//TODO: Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;

		angle += dt * kFloatPi * 0.3f;
		if (angle >= 2.f * kFloatPi)
			angle -= 2.f * kFloatPi;

		/* Update camera state
		 * Radius controls vertical distance from cameraTarget point (here: origin)
		 * Phi controls horizontal angle around the cameraTarget point
		 * Theta controls vertical angle from the horizontal plane
		 */

		// CITE: https://learnopengl.com/Getting-started/Camera
		Vec3f forward;
		forward.x = std::cos(state.camControl.yaw) * std::cos(state.camControl.pitch);
		forward.y = std::sin(state.camControl.pitch);
		forward.z = std::sin(state.camControl.yaw) * std::cos(state.camControl.pitch);
		forward = normalize(forward);

		// Calculate right vector (perpendicular to forward and world up)
		Vec3f worldUp = {0.f, 1.f, 0.f};

		/* Note: that we normalize the resulting right vector.
		 * If we wouldn't normalize this vector, the resulting cross product may return
		 *	differently sized vectors based on the cameraFront variable.

		 * If we would not normalize the vector we would move slow or fast
		 *	based on the camera's orientation instead of at a consistent movement speed.
		 */
		Vec3f right = normalize(cross(forward, worldUp));

		// Calculate actual up vector (perpendicular to forward and right)
		Vec3f up = normalize(cross(right, forward));

		if (state.camControl.cameraActive)
		{
			// Calculate movement speed with modifiers
			float speed = state.camControl.currentSpeed * dt;

			// Apply movement
			if (state.camControl.moveForward)
				state.camControl.position += forward * speed;

			if (state.camControl.moveBackward)
				state.camControl.position -= forward * speed;
			if (state.camControl.moveLeft)
				state.camControl.position -= right * speed;
			if (state.camControl.moveRight)
				state.camControl.position += right * speed;

			// Clamp vertical movement so we don't go below the floor or too high
			if (state.camControl.position.y < 0.1f)
				state.camControl.position.y = 0.1f;
			if (state.camControl.position.y > 50.f)
				state.camControl.position.y = 50.f;

			if (state.camControl.moveUp)
				state.camControl.position += up * speed;

			if (state.camControl.moveDown)
				state.camControl.position -= up * speed;
		}
		//////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////
		// TODO: Setup camera pipeline
		// 1. Model to World matrices
		// Cube above terrain, spinning
		Mat44f yRotateCCW = make_rotation_y(-angle);
		Mat44f yTranslateAboveTerrain = make_translation({0.f, 2.f, 0.f}); // 2 units above origin
		Mat44f model2world_cube = yTranslateAboveTerrain * yRotateCCW;
		Mat33f normalMatrix_cube = make_uniform_normal(model2world_cube);

		// Parlahti terrain fixed at origin, scaled down
		Mat44f scaleThreeQuarterSize = make_scaling(0.75f, 0.75f, 0.75f);
		Mat44f model2world_parlahti = scaleThreeQuarterSize;
		// Compute the normal matrix and pass it to the shaders as a uniform matrix3
		Mat33f normalMatrix_parlahti = make_uniform_normal(model2world_parlahti);

		// 2. View matrix - position and orientation based on yaw/pitch
		// Looks along forward vector
		Vec3f cameraTarget = state.camControl.position + forward;
		Mat44f world2camera_fps = make_look_at( state.camControl.position, cameraTarget, worldUp );

		// 3. Perspective projection
		Mat44f projection = make_perspective_projection( kFloatPi / 4.f, fbwidth / fbheight, 0.1f, 100.f );
		Mat44f projCamera = projection * world2camera_fps;

		// 4. Combined matrices for projection, camera and world
		Mat44f projCameraWorld_parlahti = projCamera * model2world_parlahti;
		Mat44f projCameraWorld_cube = projCamera * model2world_cube;
		//////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////
		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		// Clear buffers every frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE); // Disable face culling for debugging
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe mode for debugging

		// Next, bind the appropriate program we want to draw with our program.
		glUseProgram(unifiedProg.programId());

		// TODO: Draw frame
		// TODO: Apply to all objects added.
		// Set light uniforms
		Vec3f lightDir = normalize(Vec3f{0.f, 1.f, -1.f});
		glUniform3fv(2, 1, &lightDir.x);
		glUniform3f(3, 0.9f, 0.9f, 0.6f);	 // Location 3 (light diffuse)
		glUniform3f(4, 0.05f, 0.05f, 0.05f); // Location 4 (scene ambient)

		/* Parlahti Terrain */
		glUniform1i(10, parlahtiMesh.materialType);
		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld_parlahti.v);
		glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix_parlahti.v);

		// If material is textured, bind the texture
		if (parlahtiMesh.materialType == 1 && parlahtiTexture != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, parlahtiTexture);
		}
		glBindVertexArray(parlahtiVao);
		glDrawArrays(GL_TRIANGLES, 0, parlahtiVertexCount);

		/* Cube Mesh */
		glUniform1i(10, 0); // uMaterialType = 0
		glBindTexture(GL_TEXTURE_2D, 0); // No texture
		glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix_cube.v);
		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld_cube.v);
		glBindVertexArray(cubeVao);
		glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount); // 36 for cube

		// Reset mode and state for next frame
		// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// glEnable(GL_CULL_FACE);

		// Cleanup the modified global state: Reset VAO and program.
		glBindVertexArray(0);
		glUseProgram(0);

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// TODO: additional cleanup
	state.prog = nullptr;

	// Cleanup of OpenGL objects
	glDeleteVertexArrays(1, &parlahtiVao);
	if (parlahtiTexture != 0)
		glDeleteTextures(1, &parlahtiTexture);
	glDeleteVertexArrays(1, &cubeVao);

	return 0;
	//////////////////////////////////////////////////////////////////////////////////////
}
catch( std::exception const& eErr )
{
	std::print( stderr, "Top-level Exception ({}):\n", typeid(eErr).name() );
	std::print( stderr, "{}\n", eErr.what() );
	std::print( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::print( stderr, "GLFW error: {} ({})\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}

		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			// R-key reloads shaders.
			if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
			{
				if (state->prog)
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
				}
			}

			/* // TODO: Remove later? The user should be able to move by default.
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;
				std::print("Camera movement: {}\n", state->camControl.cameraActive ? "ENABLED" : "DISABLED");

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			*/

			// SHIFT+CTRL Speed modifiers
			if (GLFW_KEY_LEFT_SHIFT == aKey || GLFW_KEY_RIGHT_SHIFT == aKey)
			{
				state->camControl.currentSpeed = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction)
				? state->camControl.baseSpeed * 5.f // 5x faster with Shift
				: state->camControl.baseSpeed;
				std::print("Shift pressed\n");

				if (GLFW_RELEASE == aAction)
					std::print("Shift released\n");
			}
			else if (GLFW_KEY_LEFT_CONTROL == aKey || GLFW_KEY_RIGHT_CONTROL == aKey)
			{
				state->camControl.currentSpeed = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction)
				? state->camControl.baseSpeed * 0.2f // 5x slower with Ctrl
				: state->camControl.baseSpeed;
				std::print("Ctrl pressed\n");

				if (GLFW_RELEASE == aAction)
					std::print("Ctrl released\n");
			}

			// WASD+EQ Movement controls (when camera is enabled)
			if (state->camControl.cameraActive)
			{
				// If action is PRESS or REPEAT (E.g., the key is held down), set movement flag to true; else false
				// The REPEAT action allows for multi-key presses to be recognised.
				// So, moving forwards and left at the same time is possible
				if (GLFW_KEY_W == aKey)
					state->camControl.moveForward = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction);
				else if (GLFW_KEY_S == aKey)
					state->camControl.moveBackward = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction);
				else if (GLFW_KEY_A == aKey)
					state->camControl.moveLeft = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction);
				else if (GLFW_KEY_D == aKey)
					state->camControl.moveRight = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction);
				else if (GLFW_KEY_E == aKey)
					state->camControl.moveUp = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction);
				else if (GLFW_KEY_Q == aKey)
					state->camControl.moveDown = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction);
			}

			// TODO: Remove later: Print coordinates for debugging
			if (GLFW_KEY_C == aKey && GLFW_PRESS == aAction)
			{
				std::print("Camera position: ({}, {}, {})\n",
						   state->camControl.position.x,
						   state->camControl.position.y,
						   state->camControl.position.z);
			}
		}
	}

	// TODO: Can this be combined in callback motion?
	// Mouse button callback to toggle mouse look mode
	void glfw_callback_mouse_button_(GLFWwindow *aWindow, int aButton, int aAction, int)
	{
		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			if (aButton == GLFW_MOUSE_BUTTON_RIGHT && GLFW_PRESS == aAction)
			{
				// Toggle mouse look mode
				state->camControl.mouseLookActive = !state->camControl.mouseLookActive;

				if (state->camControl.mouseLookActive)
				{
					// Enable mouse look mode, hide cursor
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					state->camControl.firstMouse = true;
				}
				else
				{
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				// Console output
				std::print("Mouse look: {}\n", state->camControl.mouseLookActive ? "ENABLED" : "DISABLED");
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow *aWindow, double aX, double aY)
	{
		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.mouseLookActive)
			{
				// Initialize on first mouse movement
				if (state->camControl.firstMouse)
				{
					state->camControl.lastX = float(aX);
					state->camControl.lastY = float(aY);
					state->camControl.firstMouse = false; // Toggle off
				}

				// Calculate the mouse's offset since the last frame.
				float xoffset = float(aX) - state->camControl.lastX;
				float yoffset = state->camControl.lastY - float(aY); // Reversed for natural mouse

				state->camControl.lastX = float(aX);
				state->camControl.lastY = float(aY);

				xoffset *= state->camControl.mouseSensitivity;
				yoffset *= state->camControl.mouseSensitivity;

				// Add the offset values to the camera's yaw and pitch values.
				state->camControl.yaw += xoffset;
				state->camControl.pitch += yoffset;

				/* Constrain pitch to prevent flipping
				 * The pitch needs to be constrained in such a way that users won't be able to
				 * 	look higher than 89 degrees (at 90 degrees we get the LookAt flip) and
				 * 	also not below -89 degrees.
				 * This ensures the user will be able to look up to the sky or below to his feet but not further.
				 * The constraints work by replacing the Euler value
				 * 	with its constraint value whenever it breaches the constraint:
				 */
				// TODO: Adjust limit
				float kRadianLimit = kFloatPi / 2.1f;
				if (state->camControl.pitch > kRadianLimit)
					state->camControl.pitch = kRadianLimit;
				if (state->camControl.pitch < -kRadianLimit)
					state->camControl.pitch = -kRadianLimit;

				// Constrain yaw to the range [-pi, pi] for numerical stability
				if (state->camControl.yaw > kFloatPi)
					state->camControl.yaw -= 2.f * kFloatPi;
				else if (state->camControl.yaw < -kFloatPi)
					state->camControl.yaw += 2.f * kFloatPi;
			}
			else
			{
				// If mouse look is not active, simply update last positions
				state->camControl.lastX = float(aX);
				state->camControl.lastY = float(aY);
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
		if( window )
			glfwDestroyWindow( window );
	}
}
