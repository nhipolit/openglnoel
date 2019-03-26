#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include "Application.hpp"
#include <iostream>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians){
	const auto cosPhi = glm::cos(phiRadians);
	const auto sinPhi = glm::sin(phiRadians);
	const auto sinTheta = glm::sin(thetaRadians);
	return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
}

int Application::run(){
  	// Put here code to run before rendering loop
    glClearColor(1,1,1,1);
  	// Loop until the user closes the window
  	for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount){
      	const auto seconds = glfwGetTime();

      	// Put here rendering code
      	const auto fbSize = m_GLFWHandle.framebufferSize();
      	glViewport(0, 0, fbSize.x, fbSize.y);
      	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      	const auto sceneDiagonalSize = glm::length(_scene.bboxMax - _scene.bboxMin);
      	viewController.setSpeed(sceneDiagonalSize * 0.1f); 

      	glm::mat4 ProjMatrix = glm::perspective(70.f, float(fbSize.x) / fbSize.y, 0.01f * sceneDiagonalSize, sceneDiagonalSize);
      	glm::mat4 viewMatrix = viewController.getViewMatrix();


      	/*Light----------------------------------------------------------------*/
      	glm::vec3 lightDirection = computeDirectionVector(glm::radians(90.0f), glm::radians(45.0f));

      	glUniform3fv(uDirectionalLightDir, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(lightDirection), 0))));
      	glUniform3fv(uDirectionalLightIntensity, 1, glm::value_ptr(glm::vec3(1, 1, 1) * 1.0f));
      	glUniform3fv(uPointLightPosition, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::vec3(0, 1, 0), 1))));
      	glUniform3fv(uPointLightIntensity, 1, glm::value_ptr(glm::vec3(0, 1, 0) * 5.0f));

      	glUniform1i(uKaSampler, 0);
      	glUniform1i(uKdSampler, 1);
      	glUniform1i(uKsSampler, 2);
      	glUniform1i(uShininessSampler, 3);

      	/*Sphere----------------------------------------------------------------------------*/
      	glm::mat4 MVMatrix = viewMatrix*glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-5));
      	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
      	glm::mat4 MVPMatrix = ProjMatrix * MVMatrix;
    
      	glUniformMatrix4fv(uMVPMatrix,1,GL_FALSE,glm::value_ptr(MVPMatrix));
      	glUniformMatrix4fv(uMVMatrix,1,GL_FALSE,glm::value_ptr(MVMatrix));
      	glUniformMatrix4fv(uNormalMatrix,1,GL_FALSE,glm::value_ptr(NormalMatrix));

      	glBindVertexArray(_vao);
      	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,_FBO);
      	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      	auto indexOffset = 0;
      	int shapeIdx = 0;
      	for (const auto indexCount: _scene.indexCountPerShape){
          	if(_scene.materialIDPerShape[shapeIdx]==-1)
            	glBindTexture(GL_TEXTURE_2D,whiteTextureId);
          	else{
            	const auto& material = _scene.materials[_scene.materialIDPerShape[shapeIdx]];

            	glUniform3fv(uKa, 1, glm::value_ptr(material.Ka));
            	glUniform3fv(uKd, 1, glm::value_ptr(material.Kd));
            	glUniform3fv(uKs, 1, glm::value_ptr(material.Ks));
            	glUniform1f(uShininess,material.shininess);

            	auto idTexKa = material.KaTextureId;
            	auto idTexKd = material.KdTextureId;
            	auto idTexKs = material.KsTextureId;
            	auto idTexShininess = material.shininessTextureId;
            
            	if(idTexKa!=-1){
              		glActiveTexture(GL_TEXTURE0);
              		glBindTexture(GL_TEXTURE_2D,objectsTextureId[idTexKa]);
            	}
            	else{
              		glActiveTexture(GL_TEXTURE0);
              		glBindTexture(GL_TEXTURE_2D,0);
            	}

            	if(idTexKd!=-1){
              		glActiveTexture(GL_TEXTURE1);
              		glBindTexture(GL_TEXTURE_2D,objectsTextureId[idTexKd]);
            	}
            	else{
              		glActiveTexture(GL_TEXTURE1);
              		glBindTexture(GL_TEXTURE_2D,0);
            	}

            	if(idTexKs!=-1){
              		glActiveTexture(GL_TEXTURE2);
              		glBindTexture(GL_TEXTURE_2D,objectsTextureId[idTexKs]);
            	}
            	else{
              		glActiveTexture(GL_TEXTURE2);
              		glBindTexture(GL_TEXTURE_2D,0);
            	}

            	if(idTexShininess!=-1){
              		glActiveTexture(GL_TEXTURE3);
              		glBindTexture(GL_TEXTURE_2D,objectsTextureId[idTexShininess]);
            	}
            	else{
              		glActiveTexture(GL_TEXTURE3);
              		glBindTexture(GL_TEXTURE_2D,0);
            	}
          	}
          	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
          	indexOffset += indexCount;
          	shapeIdx++;
      	}

      	glActiveTexture(GL_TEXTURE0);
      	glBindTexture(GL_TEXTURE_2D,0);
      	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);

      	glBindFramebuffer(GL_READ_FRAMEBUFFER,_FBO);
      	glReadBuffer(GL_COLOR_ATTACHMENT0 + GNormal);
      	glBlitFramebuffer(0, 0, fbSize.x, fbSize.y, 0, 0, fbSize.x, fbSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

      	glBindFramebuffer(GL_READ_FRAMEBUFFER,0);

      	// GUI code:
      	glmlv::imguiNewFrame();

      	{
    		ImGui::Begin("GUI");
    		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    		ImGui::End();
      	}

      	glmlv::imguiRenderFrame();

      	glfwPollEvents(); // Poll for and process events

      	m_GLFWHandle.swapBuffers(); // Swap front and back buffers

      	auto ellapsedTime = glfwGetTime() - seconds;
      	auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
      	if (!guiHasFocus)
        	viewController.update(float(ellapsedTime));

    }
  	return 0;
}

Application::Application(int argc, char** argv):
  	m_AppPath { glmlv::fs::path{ argv[0] } },
  	m_AppName { m_AppPath.stem().string() },
  	m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
  	m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
  	viewController{ m_GLFWHandle.window() }
{
  	shader = glmlv::compileProgram({m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
  	ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

  	this->recupMatricesId(&shader);
  	loadObjScene(argv[1], _scene);
  	shader.use();
  	glEnable(GL_DEPTH_TEST);

  	this->initVboVao();
  	//Section to new textures-----------------------------------------------------------------
  	glGenTextures(GBufferTextureCount, m_GBufferTextures);

  	const auto fbSize = m_GLFWHandle.framebufferSize();
  	const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };

  	for (int32_t i = GPosition; i < GBufferTextureCount; i++){
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], fbSize.x, fbSize.y);
  	}
  
  	glGenFramebuffers(1,&_FBO);
  	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,_FBO);
  	for (int32_t i = GPosition; i <= GGlossyShininess; i++)
    	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
  	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GDepth], 0);

  	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
  	glDrawBuffers(5, drawBuffers);
  	glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

  	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
  	//-----------------------------------------------------------------------------------------


  	glGenTextures(1, &whiteTextureId);
        glBindTexture(GL_TEXTURE_2D, whiteTextureId);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
        glm::vec4 whiteTexture(1.f, 1.f, 1.f, 1.f);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &whiteTexture);
  	glBindTexture(GL_TEXTURE_2D, 0);
  
  	for(const auto& texture : _scene.textures){
    	GLuint texId = 0;
    	glGenTextures(1,&texId);
    	glBindTexture(GL_TEXTURE_2D, texId);
        	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texture.width(),texture.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture.data());
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glBindTexture(GL_TEXTURE_2D, 0);

    	objectsTextureId.push_back(texId);
  	}

}

void Application::initVboVao(){
  //VBO
  glGenBuffers(1,&_vbo);
  glBindBuffer(GL_ARRAY_BUFFER,_vbo);
  glBufferStorage(GL_ARRAY_BUFFER,_scene.vertexBuffer.size()*sizeof(glmlv::Vertex3f3f2f),_scene.vertexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  //IBO
  glGenBuffers(1,&_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_ibo);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,_scene.indexBuffer.size()*sizeof(uint32_t),_scene.indexBuffer.data(),GL_DYNAMIC_STORAGE_BIT);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  
  //VAO
  glGenVertexArrays(1,&_vao);
  glBindVertexArray(_vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  
  glBindVertexBuffer(0,_vbo,0,sizeof(glmlv::Vertex3f3f2f));
  
  glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
  glVertexAttribFormat(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, position));
  glVertexAttribBinding(VERTEX_ATTR_POSITION, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
  glVertexAttribFormat(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, normal));
  glVertexAttribBinding(VERTEX_ATTR_NORMAL, 0);
  glEnableVertexAttribArray(VERTEX_ATTR_TEXTURE);
  glVertexAttribFormat(VERTEX_ATTR_TEXTURE, 2, GL_FLOAT, GL_FALSE, offsetof(glmlv::Vertex3f3f2f, texCoords));
  glVertexAttribBinding(VERTEX_ATTR_TEXTURE, 0);
  
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindVertexArray(0);
}

void Application::recupMatricesId(glmlv::GLProgram* shader){
  	uMVPMatrix = shader->getUniformLocation("uMVPMatrix");
  	uMVMatrix = shader->getUniformLocation("uMVMatrix");
  	uNormalMatrix = shader->getUniformLocation("uNormalMatrix");
  
  	uDirectionalLightDir = shader->getUniformLocation("uDirectionalLightDir");
  	uDirectionalLightIntensity = shader->getUniformLocation("uDirectionalLightIntensity");
  	uPointLightPosition = shader->getUniformLocation("uPointLightPosition");
  	uPointLightIntensity = shader->getUniformLocation("uPointLightIntensity");

  	uKaSampler = shader->getUniformLocation("uKaSampler");
  	uKdSampler = shader->getUniformLocation("uKdSampler");
  	uKsSampler = shader->getUniformLocation("uKsSampler");
  	uShininessSampler = shader->getUniformLocation("uShininessSampler");

  	uKa = shader->getUniformLocation("uKa");
  	uKd = shader->getUniformLocation("uKd");
  	uKs = shader->getUniformLocation("uKs");
  	uShininess = shader->getUniformLocation("uShininess");
}