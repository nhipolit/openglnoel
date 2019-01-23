#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int Application::run(){
	// Put here code to run before rendering loop
    glm::vec3 kd;
    glm::vec3 lightDir;
    glm::vec3 lightIntensity;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount){
        const auto seconds = glfwGetTime();

        // Put here rendering code
		const auto fbSize = m_GLFWHandle.framebufferSize();
		glViewport(0, 0, fbSize.x, fbSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        kd = glm::vec3(1,1,1);
        lightDir = glm::vec3(0,2,1);
        lightIntensity = glm::vec3(1,1,1);

        glUniform3fv(uKd, 1, glm::value_ptr(kd));
        glUniform3fv(uDirectionLightDir, 1, glm::value_ptr(lightDir));
        glUniform3fv(uDirectionalLightIntensity, 1, glm::value_ptr(lightIntensity));

        // CUBE
        glUniform1i(uKdSampler,0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textCube);

        glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f),(float)fbSize.x/fbSize.y, 0.1f, 100.f);
        glm::mat4 ViewMatrix = viewController.getViewMatrix();
        glm::mat4 ModelViewMatrix = ViewMatrix * glm::translate(glm::mat4(1.f), glm::vec3(2.5,0,-5));
        ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1.55f,1.55f,1.55f));
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ModelViewMatrix));
        glm::mat4 MVPMatrix = ProjMatrix * ModelViewMatrix;

        glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
        glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(ModelViewMatrix));
        glUniformMatrix4fv(uNMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));

        glBindVertexArray(vao[0]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
                glDrawElements(GL_TRIANGLES, nbIndexCube, GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

        // SPHERE
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textSphere);

        ModelViewMatrix = ViewMatrix * glm::translate(glm::mat4(1.f), glm::vec3(0,0,-5));
        NormalMatrix = glm::transpose(glm::inverse(ModelViewMatrix));
        MVPMatrix = ProjMatrix * ModelViewMatrix;

        glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
        glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(ModelViewMatrix));
        glUniformMatrix4fv(uNMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));

        glBindVertexArray(vao[1]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
                glDrawElements(GL_TRIANGLES, nbIndexSphere, GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

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
            viewController.update(ellapsedTime);
        }

		m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath{glmlv::fs::path{argv[0]}},
    m_AppName{m_AppPath.stem().string()},
    m_ImGuiIniFilename {m_AppName + ".imgui.ini"},
    m_ShadersRootPath {m_AppPath.parent_path() / "shaders"},
    viewController{m_GLFWHandle.window()}
    {
        shader = glmlv::compileProgram({m_ShadersRootPath / m_AppName / "forward.vs.glsl",
            m_ShadersRootPath / m_AppName / "forward.fs.glsl"});
        ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

        glmlv::Image2DRGBA imgCube = glmlv::readImage("/home/2ins2/nhipolit/Documents/Semestre2/synthese/openglnoel/apps/forward-renderer/assets/companion.jpg");
        glmlv::Image2DRGBA imgSphere = glmlv::readImage("/home/2ins2/nhipolit/Documents/Semestre2/synthese/openglnoel/apps/forward-renderer/assets/star.png");

        // CUBE
        glGenTextures(1,&textCube);
        glBindTexture(GL_TEXTURE_2D, textCube);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,imgCube.width(),imgCube.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,imgCube.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        //Sphere
        glGenTextures(1,&textSphere);
        glBindTexture(GL_TEXTURE_2D, textSphere);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,imgSphere.width(),imgSphere.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,imgSphere.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        uMVPMatrix = shader.getUniformLocation("uModelViewProjMatrix");
        uMVMatrix = shader.getUniformLocation("uModelViewMatrix");
        uNMatrix = shader.getUniformLocation("uNormalMatrix");
        uDirectionLightDir = shader.getUniformLocation("uDirectionLightDir");
        uDirectionalLightIntensity = shader.getUniformLocation("uDirectionalLightIntensity");
        uPointLightPosition = shader.getUniformLocation("uPointLightPosition");
        uPointLightIntensity = shader.getUniformLocation("uPointLightIntensity");
        uKd = shader.getUniformLocation("uKd");
        uKdSampler = shader.getUniformLocation("uKdSampler");
        shader.use();

        glmlv::SimpleGeometry cube = glmlv::makeCube();
        glmlv::SimpleGeometry sphere = glmlv::makeSphere(20);

        /********************************************/

        GLuint vbo[2];
        glGenBuffers(2, vbo);
        
        // CUBE
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            glBufferStorage(GL_ARRAY_BUFFER, cube.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), cube.vertexBuffer.data(), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // SPHERE
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
            glBufferStorage(GL_ARRAY_BUFFER, sphere.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), sphere.vertexBuffer.data(), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /********************************************/

        glGenBuffers(2, ibo);

        // CUBE
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
            nbIndexCube = cube.indexBuffer.size();
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, nbIndexCube*sizeof(uint32_t), cube.indexBuffer.data(), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // SPHERE
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[1]);
            nbIndexSphere = sphere.indexBuffer.size();
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, nbIndexSphere*sizeof(uint32_t), sphere.indexBuffer.data(), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        /********************************************/

        glGenVertexArrays(2, vao);

        const GLuint VERTEX_ATTR_POSITION = 0;
        const GLuint NORMAL_ATTR_POSITION = 1;
        const GLuint TEX_ATTR_POSITION = 2;

        // CUBE
        glBindVertexArray(vao[0]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
                glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
                glEnableVertexAttribArray(NORMAL_ATTR_POSITION);
                glEnableVertexAttribArray(TEX_ATTR_POSITION);
                glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
                    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), 0);
                    glVertexAttribPointer(NORMAL_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f,normal));
                    glVertexAttribPointer(TEX_ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f,texCoords));
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // SPHERE
        glBindVertexArray(vao[1]);
            glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
            glEnableVertexAttribArray(NORMAL_ATTR_POSITION);
            glEnableVertexAttribArray(TEX_ATTR_POSITION);
            glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
                glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), 0);
                glVertexAttribPointer(NORMAL_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f,normal));
                glVertexAttribPointer(TEX_ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f,texCoords));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);
    }