#ifndef TEST_HPP
#define TEST_HPP

#include <glad/glad.h>

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "simple_mesh.hpp"
#include "defaults.hpp"
#include "cone.hpp"
#include "cylinder.hpp"
#include "texture.hpp"
#include "loadobj.hpp"
#include "landing_pad.hpp"
#include "renderer.hpp"
#include "test.hpp"


void test_vao_creation();

void test_all_mesh_functions();

#endif // TEST_HPP