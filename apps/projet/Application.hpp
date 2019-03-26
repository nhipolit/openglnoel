#pragma once
#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/scene_loading.hpp>
#include <glmlv/ViewController.hpp>
#include <iostream>

using namespace std;
const GLint VERTEX_ATTR_POSITION = 0;
const GLint VERTEX_ATTR_NORMAL = 1;
const GLint VERTEX_ATTR_TEXTURE = 2;


class Application{
	public:
  		Application(int argc, char** argv);

  		enum GBufferTextureType{
    		GPosition = 0,
    		GNormal,
    		GAmbient,
    		GDiffuse,
    		GGlossyShininess,
    		GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
    		GBufferTextureCount
  		};

  		int run();
  		void initVboVao();
  		void recupMatricesId(glmlv::GLProgram* shader);

	private:
  		struct SceneObject{
    		uint32_t nbVertices;
    		uint32_t indexFirstVertex;
    		int localMatrixIndex;
    		int32_t materialIndex = -1;
  		};

  		const size_t m_nWindowWidth = 1280;
  		const size_t m_nWindowHeight = 720;
  		glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

  		const glmlv::fs::path m_AppPath;
  		const std::string m_AppName;
  		const std::string m_ImGuiIniFilename;
  		const glmlv::fs::path m_ShadersRootPath;

  		glmlv::ViewController viewController;

  		glmlv::Image2DRGBA imgTextureSphere;
  		GLuint _vbo,_vao,_ibo,texture,whiteTextureId;

  		glmlv::GLProgram shader;
  
  		GLint uMVPMatrix;
  		GLint uMVMatrix;
  		GLint uNormalMatrix;

  		GLint uDirectionalLightDir;
  		GLint uDirectionalLightIntensity;
  		GLint uPointLightPosition;
  		GLint uPointLightIntensity;

  		GLint uKaSampler;
  		GLint uKdSampler;
  		GLint uKsSampler;
  		GLint uShininessSampler;
  		GLint uKa;
  		GLint uKd;
  		GLint uKs;
  		GLint uShininess;

  		glmlv::SceneData _scene;
  		std::vector<GLuint> objectsTextureId;
  		//Deferred--------------------------------------------------------------
  		GLuint fPosition;
  		GLuint fNormal;
  		GLuint fAmbient;
  		GLuint fDiffuse;
  		GLuint fGlossyShininess;

  		GLuint _FBO;

  		GLuint m_GBufferTextures[GBufferTextureCount];
};