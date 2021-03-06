#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/scene_loading.hpp>


class Application{
	public:
	    Application(int argc, char** argv);
	    int run();

	private:
            struct SceneObject{
                uint32_t nbVertices;
                uint32_t indexFirstVertex;
                int localMatrixIndex;
                int32_t materialIndex = -1;
            };
	    const size_t m_nWindowWidth = 1280;
	    const size_t m_nWindowHeight = 720;
	    glmlv::GLFWHandle m_GLFWHandle{m_nWindowWidth, m_nWindowHeight, "Template"}; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

	    const glmlv::fs::path m_AppPath;
	    const std::string m_AppName;
	    const std::string m_ImGuiIniFilename;
	    const glmlv::fs::path m_ShadersRootPath;

	    GLuint vao;
        GLuint ibo;

        glmlv::GLProgram shader;
        GLuint uMVPMatrix;
        GLuint uMVMatrix;
        GLuint uNMatrix;

        GLuint uDirectionLightDir;
        GLuint uDirectionalLightIntensity;
        GLuint uPointLightPosition;
        GLuint uPointLightIntensity;
        GLuint uKd;
        GLuint uKdSampler;

        glmlv::ViewController viewController;

        glmlv::SceneData scene;
        GLuint whiteTextureId;
        std::vector<GLuint> objectsTextureId;

        GLuint uKa;
        GLuint uKaSampler;
        GLuint uKs;
        GLuint uKsSampler;
        GLuint uShininess;
        GLuint uShininessSampler;
};