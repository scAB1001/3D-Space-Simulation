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

#include "defaults.hpp"
#include "cube.hpp" // TODO: Remove if not using cube data

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";
	// TODO: Constants for camera control
	constexpr float kMovementPerSecond_ = 5.f;	// units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	// TODO: State struct with camctrl_
	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut;

			float phi, theta;
			float radius;

			float fov;

			float lastX, lastY;
		} camControl;
	};

	void glfw_callback_error_( int, char const* );
	void glfw_callback_scroll_(GLFWwindow *, double, double);
	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
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

// TODO: Vertex struct for shapes
struct Vertex
{
	float position[2];		// 2x4 (float) = 8 bytes
	unsigned char color[3]; // 3 bytes
	unsigned char padding;	// For 4-byte alignment (optional but good practice)
};

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
	// TODO: Additional event handling setup
	// TODO: Setup the state struct with camctrl_
	State_ state{};

	// Setup the additional GLFW callbacks
	glfwSetWindowUserPointer( window, &state );
	glfwSetKeyCallback( window, &glfw_callback_key_ );
	glfwSetScrollCallback(window, &glfw_callback_scroll_);
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

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: global GL setup goes here
	// Enable sRGB framebuffer - we want correct gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);

	// Enable face culling - we want to avoid drawing back faces
	glEnable(GL_CULL_FACE);
	// This is the default, but be explicit
	glCullFace(GL_BACK);

	// Define front faces to be counter-clockwise
	glFrontFace(GL_CCW);

	// Enable depth testing - we want correct occlusion
	glEnable(GL_DEPTH_TEST);
	// Use less-than depth test, which is the default but be explicit
	glDepthFunc(GL_LESS);
	// Set clear depth to far plane (1.0) to match depth test
	// TODO: glad_glDepthRangef(0.f, 1.f);
	glClearDepthf(1.f);

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f); // dark gray - values between 0.0 and 1.0
	//////////////////////////////////////////////////////////////////////////////////

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// TODO: Setup shader program
	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
	} );

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: global GL setup goes here
	// Set up initial state
	state.prog = &prog;
	state.camControl.radius = 10.f;
	state.camControl.fov = std::numbers::pi_v<float> / 4.f; // 45 degrees

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;
	//////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////
	// TODO: Setup scene objects e.g. VAOs, VBOs, textures, uniforms, etc.
	GLuint vbo; // Init to 0?
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	/* Triangle example
	static const Vertex allVertices[] = {
		// Triangle 1
		{{0.0f, 0.8f}, {255, 255, 0}, 0},
		{{-0.7f, -0.8f}, {255, 0, 255}, 0},
		{{0.7f, -0.8f}, {0, 255, 255}, 0},

		// Triangle 2
		{{-0.75f, 0.8f}, {0, 255, 255}, 0},
		{{-0.8f, -0.8f}, {0, 255, 255}, 0},
		{{-0.1f, 0.8f}, {0, 0, 0}, 0},

		// Triangle 3
		{{0.8f, -0.8f}, {0, 255, 255}, 0},
		{{0.75f, 0.8f}, {0, 255, 255}, 0},
		{{0.1f, 0.8f}, {0, 0, 0}, 0}};
	glBufferData(GL_ARRAY_BUFFER, sizeof(allVertices), allVertices, GL_STATIC_DRAW);
	*/

	/* Cube example */
	size_t vertexCount = std::size(kCubePositions) / 3;
	std::vector<Vertex> allVertices(vertexCount);

	for (size_t i = 0; i < vertexCount; ++i)
	{
		allVertices[i].position[0] = kCubePositions[i * 3 + 0];
		allVertices[i].position[1] = kCubePositions[i * 3 + 1];
		allVertices[i].position[2] = kCubePositions[i * 3 + 2];

		allVertices[i].color[0] = static_cast<unsigned char>(kCubeColors[i * 3 + 0] * 255.f);
		allVertices[i].color[1] = static_cast<unsigned char>(kCubeColors[i * 3 + 1] * 255.f);
		allVertices[i].color[2] = static_cast<unsigned char>(kCubeColors[i * 3 + 2] * 255.f);
	}
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(Vertex), allVertices.data(), GL_STATIC_DRAW);
	//////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glVertexAttribPointer(
		0,				// Position attribute 0 in the shader
		3, GL_FLOAT,	// Three floats per position
		GL_FALSE,		// Not normalized
		sizeof(Vertex), // Stride: Size of a single Vertex
		(void *)0		// Offset to position data in Vertex struct
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1, // Color attribute 1 in the shader
		3, GL_UNSIGNED_BYTE,
		GL_TRUE, // Normalize to [0.0, 1.0] in shader as we use unsigned bytes for color
		sizeof(Vertex),
		(void *)offsetof(Vertex, color) // Offset to color data in Vertex struct
	);
	glEnableVertexAttribArray(1);

	// Reset state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//////////////////////////////////////////////////////////////////////////////////

	OGL_CHECKPOINT_ALWAYS();

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
		// Update state
		//TODO: update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;

		angle += dt * std::numbers::pi_v<float> * 0.3f;
		if (angle >= 2.f * std::numbers::pi_v<float>)
			angle -= 2.f * std::numbers::pi_v<float>;

		// Update camera state
		if (state.camControl.actionZoomIn)
			state.camControl.radius -= kMovementPerSecond_ * dt;
		else if (state.camControl.actionZoomOut)
			state.camControl.radius += kMovementPerSecond_ * dt;

		if (state.camControl.radius <= 0.1f)
			state.camControl.radius = 0.1f;
		//////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////
		// TODO: Setup camera pipeline
		// 1. Model to world
		Mat44f model2world_cube1 = make_rotation_y(angle);
		// Translate the cube to orbit position (3 units to the right)
		Mat44f translate_to_orbit = make_translation({3.f, 0.f, 0.f});
		// Then rotate it around the origin
		Mat44f orbit_rotation = make_rotation_y(angle);
		// Combine: orbit rotation first, then translation
		Mat44f model2world_cube2 = orbit_rotation * translate_to_orbit;

		// 2. World to camera (fps-style camera)
		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Vec3f camTranslation = {0.f, 0.f, -state.camControl.radius};
		Mat44f T = make_translation(camTranslation);
		Mat44f world2camera_fps = Rx * Ry * T;

		// 3. Perspective Projection
		Mat44f projection = make_perspective_projection(
			state.camControl.fov,
			fbwidth / fbheight,
			0.1f, 100.f);

		// 4. Combined final matrix
		Mat44f projCameraWorld_cube1 = projection * world2camera_fps * model2world_cube1;
		Mat44f projCameraWorld_cube2 = projection * world2camera_fps * model2world_cube2;
		//////////////////////////////////////////////////////////////////////////////////
		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Next, bind the appropriate program we want to draw with our program.
		glUseProgram(prog.programId());

		//TODO: draw frame
		// Cube 1
		glUniformMatrix4fv(
			0,						// location 0 for uProjCameraWorld
			1, GL_TRUE,				// 1 matrix, transpose (row-major to column-major)
			projCameraWorld_cube1.v // pointer to matrix data
		);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36); //36 for cube, multiples of 3 for triangles

		// Cube 2
		glUniformMatrix4fv(
			0,
			1, GL_TRUE,
			projCameraWorld_cube2.v
		);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Cleanup the modified global state: Reset VAO and program.
		glBindVertexArray(0);
		glUseProgram(0);

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
		//////////////////////////////////////////////////////////////////////////////////
	}

	// TODO: additional cleanup
	state.prog = nullptr;

	// Cleanup of OpenGL objects
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

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

	void glfw_callback_scroll_(GLFWwindow *aWindow, double aX, double aY)
	{
		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			// Only zoom if camera control is not active
			if (!state->camControl.cameraActive)
			{
				// Scroll up (aY > 0): zoom in (decrease radius)
				// Scroll down (aY < 0): zoom out (increase radius)
				state->camControl.radius -= float(aY) * 0.5f;

				std::print("Camera Distance: {:.1f} units\n", state->camControl.radius);

				// Clamp radius to prevent getting too close or negative values
				if (state->camControl.radius <= 0.1f)
					state->camControl.radius = 0.1f;

				// Optional: Add maximum distance limit
				if (state->camControl.radius > 50.f)
					state->camControl.radius = 50.f;
			}

			// If camera control is active, modify FOV
			else
			{
				// Adjust FOV based on scroll
				// Scroll up (aY > 0): Decrease FOV (zoom in)
				// Scroll down (aY < 0): Increase FOV (zoom out)
				state->camControl.fov -= float(aY) * 0.05f; // Adjust sensitivity as needed

				// Clamp FOV to reasonable values (15° to 120°)
				constexpr float minFov = 15.f * std::numbers::pi_v<float> / 180.f;	// 15° in radians
				constexpr float maxFov = 120.f * std::numbers::pi_v<float> / 180.f; // 120° in radians

				if (state->camControl.fov < minFov)
					state->camControl.fov = minFov;
				if (state->camControl.fov > maxFov)
					state->camControl.fov = maxFov;

				float fovDegrees = state->camControl.fov * 180.f / std::numbers::pi_v<float>;
				std::print("FOV: {:.1f}°\n", fovDegrees);
			}
		}
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

			// Space toggles camera
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

			// Camera controls if camera is active
			if (state->camControl.cameraActive)
			{
				if (GLFW_KEY_W == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionZoomIn = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionZoomIn = false;
				}
				else if (GLFW_KEY_S == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionZoomOut = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionZoomOut = false;
				}
			}
		}
	}

	void glfw_callback_motion_(GLFWwindow *aWindow, double aX, double aY)
	{
		if (auto *state = static_cast<State_ *>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{
				auto const dx = float(aX - state->camControl.lastX);
				auto const dy = float(aY - state->camControl.lastY);

				state->camControl.phi += dx * kMouseSensitivity_;

				state->camControl.theta += dy * kMouseSensitivity_;
				if (state->camControl.theta > std::numbers::pi_v<float> / 2.f)
					state->camControl.theta = std::numbers::pi_v<float> / 2.f;
				else if (state->camControl.theta < -std::numbers::pi_v<float> / 2.f)
					state->camControl.theta = -std::numbers::pi_v<float> / 2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
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
