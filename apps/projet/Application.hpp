#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include <GL/glew.h>

#define GLFW_INCLUDE_GLU
#include "glfw3.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#include "tiny_gltf.h"
//#include "trackball.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace tinygltf;

class Application{
	private:
		static size_t ComponentTypeByteSize(int type);
		void CheckErrors(std::string desc);
		static void SetupMeshState(tinygltf::Model &model, GLuint progId);
		static void DrawMesh(tinygltf::Model &model, const tinygltf::Mesh &mesh);
		static void DrawNode(tinygltf::Model &model, const tinygltf::Node &node);
		static void DrawModel(tinygltf::Model &model);
		bool LoadShader(GLenum shaderType,  // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER(or
                                    // maybe GL_COMPUTE_SHADER)
                GLuint &shader, const char *shaderSourceFilename);
		bool LinkShader(GLuint &prog, GLuint &vertShader, GLuint &fragShader);

	public:
		Application(int argc, char** argv);
		int run();
};