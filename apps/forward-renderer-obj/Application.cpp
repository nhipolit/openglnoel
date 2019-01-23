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


        glUniform1i(uKdSampler,0);

        glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f),(float)fbSize.x/fbSize.y, 0.1f, 2000.f);
        glm::mat4 ViewMatrix = viewController.getViewMatrix();
        glm::mat4 ModelViewMatrix = ViewMatrix * glm::translate(glm::mat4(1.f), glm::vec3(2.5,0,-5));
        ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(1.55f,1.55f,1.55f));
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(ModelViewMatrix));
        glm::mat4 MVPMatrix = ProjMatrix * ModelViewMatrix;

        glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
        glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(ModelViewMatrix));
        glUniformMatrix4fv(uNMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));

        glBindVertexArray(vao);
            auto indexOffset = 0;
            int shapeIdx = 0;
            for(const auto indexCount: scene.indexCountPerShape){
                glActiveTexture(GL_TEXTURE0);
                if(scene.materialIDPerShape[shapeIdx] == -1)
                    glBindTexture(GL_TEXTURE_2D, whiteTextureId);
                else{
                    const auto& material = scene.materials[scene.materialIDPerShape[shapeIdx]];
                    auto idTex = material.KdTextureId;
                    if(idTex!=-1)
                        glBindTexture(GL_TEXTURE_2D, objectsTextureId[idTex]);
                    else
                        glBindTexture(GL_TEXTURE_2D,0);
                }
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
                indexOffset += indexCount;
                shapeIdx++;
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
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


        uMVPMatrix = shader.getUniformLocation("uModelViewProjMatrix");
        uMVMatrix = shader.getUniformLocation("uModelViewMatrix");
        uNMatrix = shader.getUniformLocation("uNormalMatrix");
        uDirectionLightDir = shader.getUniformLocation("uDirectionLightDir");
        uDirectionalLightIntensity = shader.getUniformLocation("uDirectionalLightIntensity");
        uPointLightPosition = shader.getUniformLocation("uPointLightPosition");
        uPointLightIntensity = shader.getUniformLocation("uPointLightIntensity");
        uKd = shader.getUniformLocation("uKd");
        uKdSampler = shader.getUniformLocation("uKdSampler");

        loadObjScene("/home/2ins2/nhipolit/Documents/Semestre2/synthese/openglnoel/apps/forward-renderer-obj/assets/sponza/sponza.obj", scene);
        shader.use();

        /********************************************/

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferStorage(GL_ARRAY_BUFFER, scene.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f), scene.vertexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /********************************************/

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, scene.indexBuffer.size()*sizeof(uint32_t), scene.indexBuffer.data(), GL_DYNAMIC_STORAGE_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        /********************************************/

        glGenVertexArrays(1, &vao);

        const GLuint VERTEX_ATTR_POSITION = 0;
        const GLuint NORMAL_ATTR_POSITION = 1;
        const GLuint TEX_ATTR_POSITION = 2;


        glBindVertexArray(vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                glBindVertexBuffer(0, vbo, 0, sizeof(glmlv::Vertex3f3f2f));

                glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
                glVertexAttribFormat(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));
                glVertexAttribBinding(VERTEX_ATTR_POSITION, 0);

                glEnableVertexAttribArray(NORMAL_ATTR_POSITION);
                glVertexAttribFormat(NORMAL_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));
                glVertexAttribBinding(NORMAL_ATTR_POSITION, 0);

                glEnableVertexAttribArray(TEX_ATTR_POSITION);
                glVertexAttribFormat(TEX_ATTR_POSITION, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));
                glVertexAttribBinding(TEX_ATTR_POSITION, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);


        glGenTextures(1, &whiteTextureId);
        glBindTexture(GL_TEXTURE_2D, whiteTextureId);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
            glm::vec4 whiteTexture(1.f, 1.f, 1.f, 1.f);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &whiteTexture);
        glBindTexture(GL_TEXTURE_2D, 0);

        for(const auto& texture : scene.textures){
            GLuint texId = 0;
            glGenTextures(1,&texId);
            glBindTexture(GL_TEXTURE_2D, texId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,texture.width(), texture.height(), 0, GL_RGBA,GL_UNSIGNED_BYTE,texture.data());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            objectsTextureId.push_back(texId);
        }
    }