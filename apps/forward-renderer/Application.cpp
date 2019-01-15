#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int Application::run(){
	// Put here code to run before rendering loop

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount){
        const auto seconds = glfwGetTime();

        // Put here rendering code
		const auto fbSize = m_GLFWHandle.framebufferSize();
		glViewport(0, 0, fbSize.x, fbSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vaoCube);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
                glDrawElements(GL_TRIANGLES, nbIndexCube, GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindVertexArray(vaoSphere);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
                glDrawElements(GL_TRIANGLES, nbIndexSphere, GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
        glBindVertexArray(0);

        // GUI code:
		glmlv::imguiNewFrame();
        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

		glmlv::imguiRenderFrame();

        glfwPollEvents(); // Poll for and process events

        auto ellapsedTime = glfwGetTime()-seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            // Put here code to handle user interactions
        }

		m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath{glmlv::fs::path{argv[0]}},
    m_AppName{m_AppPath.stem().string()},
    m_ImGuiIniFilename {m_AppName + ".imgui.ini"},
    m_ShadersRootPath {m_AppPath.parent_path() / "shaders"}

    {
        ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

        glmlv::SimpleGeometry cube = glmlv::makeCube();
        glmlv::SimpleGeometry sphere = glmlv::makeSphere(10);

        int nbCube = cube.indexBuffer.size();
        int nbSphere = sphere.indexBuffer.size();

        GLuint vbo[2];
        glGenBuffers(2, vbo);
        
        // CUBE
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            const glmlv::Vertex3f3f2f *verticesCube = cube.vertexBuffer.data();
            glBufferData(GL_ARRAY_BUFFER, cube.vertexBuffer.size()*sizeof(GLfloat), verticesCube, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // SPHERE
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            const glmlv::Vertex3f3f2f *verticesSphere = sphere.vertexBuffer.data();
            glBufferData(GL_ARRAY_BUFFER, sphere.vertexBuffer.size()*sizeof(GLfloat), verticesSphere, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        glGenBuffers(2, ibo);

        // CUBE
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
            const uint32_t *indexesCube = cube.indexBuffer.data();
            nbIndexCube = cube.indexBuffer.size();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, nbCube*sizeof(uint32_t), indexesCube, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // SPHERE
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
            const uint32_t *indexesSphere = sphere.indexBuffer.data();
            nbIndexSphere = sphere.indexBuffer.size();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, nbSphere*sizeof(uint32_t), indexesSphere, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


        glGenVertexArrays(nbCube, &vaoCube);
        glGenVertexArrays(nbSphere, &vaoSphere);

        const GLuint VERTEX_ATTR_POSITION = 0;
        const GLuint NORMAL_ATTR_POSITION = 1;
        const GLuint TEX_ATTR_POSITION = 2;

        // CUBE
        glBindVertexArray(vaoCube);
            glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
            glEnableVertexAttribArray(NORMAL_ATTR_POSITION);
            glEnableVertexAttribArray(TEX_ATTR_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
                glVertexAttribPointer(NORMAL_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
                glVertexAttribPointer(TEX_ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
std::cout << "TEST 0" << std::endl;
        // SPHERE
        glBindVertexArray(vaoSphere);
            glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
            glEnableVertexAttribArray(NORMAL_ATTR_POSITION);
            glEnableVertexAttribArray(TEX_ATTR_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
                glVertexAttribPointer(NORMAL_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
                glVertexAttribPointer(TEX_ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
std::cout << "TEST 1" << std::endl;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        std::cout << "TEST 2" << std::endl;
    }