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


typedef struct {
  	GLuint vb;
} GLBufferState;

typedef struct {
  	std::vector<GLuint> diffuseTex;  // for each primitive in mesh
} GLMeshState;

typedef struct {
  GLuint vb;     // vertex buffer
  size_t count;  // byte count
} GLCurvesState;

typedef struct {
  std::map<std::string, GLint> attribs;
  std::map<std::string, GLint> uniforms;
} GLProgramState;


std::map<int, GLBufferState> gBufferState;
std::map<std::string, GLMeshState> gMeshState;
std::map<int, GLCurvesState> gCurvesMesh;
GLProgramState gGLProgramState;



static size_t ComponentTypeByteSize(int type){
	switch (type) {
	    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
	    case TINYGLTF_COMPONENT_TYPE_BYTE:
	      	return sizeof(char);
	    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
	    case TINYGLTF_COMPONENT_TYPE_SHORT:
	      	return sizeof(short);
	    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
	    case TINYGLTF_COMPONENT_TYPE_INT:
	      	return sizeof(int);
	    case TINYGLTF_COMPONENT_TYPE_FLOAT:
	      	return sizeof(float);
	    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
	      	return sizeof(double);
	    default:
	      	return 0;
	}
}

void CheckErrors(std::string desc) {
  	GLenum e = glGetError();
  	if (e != GL_NO_ERROR) {
    	fprintf(stderr, "OpenGL error in \"%s\": %d (%d)\n", desc.c_str(), e, e);
    	exit(20);
  	}
}

static void SetupMeshState(tinygltf::Model &model, GLuint progId) {
  	// Buffer{
    for(size_t i = 0 ; i < model.bufferViews.size(); i++){
      	const tinygltf::BufferView &bufferView = model.bufferViews[i];
      	if (bufferView.target == 0){
        	std::cout << "WARN: bufferView.target is zero" << std::endl;
        	continue;  // Unsupported bufferView.
      	}

      	int sparse_accessor = -1;
      	for(size_t a_i = 0 ; a_i < model.accessors.size() ; ++a_i) {
        	const auto &accessor = model.accessors[a_i];
        	if (accessor.bufferView == i) {
	          	std::cout << i << " is used by accessor " << a_i << std::endl;
	          	if (accessor.sparse.isSparse) {
	            	std::cout << "WARN: this bufferView has at least one sparse accessor to it. We are going to load the data as patched by this sparse accessor, not the original data" << std::endl;
	            	sparse_accessor = a_i;
	            	break;
	          	}
        	}
      	}

      	const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
      	GLBufferState state;
      	glGenBuffers(1, &state.vb);
      	glBindBuffer(bufferView.target, state.vb);
      	std::cout << "buffer.size= " << buffer.data.size() << ", byteOffset = " << bufferView.byteOffset << std::endl;

      	if (sparse_accessor < 0)
        	glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
      	else {
	        const auto accessor = model.accessors[sparse_accessor];
	        // copy the buffer to a temporary one for sparse patching
	        unsigned char *tmp_buffer = new unsigned char[bufferView.byteLength];
	        memcpy(tmp_buffer, buffer.data.data() + bufferView.byteOffset,
	               bufferView.byteLength);

	        const size_t size_of_object_in_buffer =
	            ComponentTypeByteSize(accessor.componentType);
	        const size_t size_of_sparse_indices =
	            ComponentTypeByteSize(accessor.sparse.indices.componentType);

	        const auto &indices_buffer_view =
	            model.bufferViews[accessor.sparse.indices.bufferView];
	        const auto &indices_buffer = model.buffers[indices_buffer_view.buffer];

	        const auto &values_buffer_view =
	            model.bufferViews[accessor.sparse.values.bufferView];
	        const auto &values_buffer = model.buffers[values_buffer_view.buffer];

	        for (size_t sparse_index = 0 ; sparse_index < accessor.sparse.count ; ++sparse_index) {
	          	int index = 0;
	          	// std::cout << "accessor.sparse.indices.componentType = " <<
	          	// accessor.sparse.indices.componentType << std::endl;
	          	switch (accessor.sparse.indices.componentType) {
	            	case TINYGLTF_COMPONENT_TYPE_BYTE:
	            	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
	              		index = (int)*(unsigned char *)
	              						(indices_buffer.data.data() +
	                                   	indices_buffer_view.byteOffset +
	                                   	accessor.sparse.indices.byteOffset +
	                                   	(sparse_index * size_of_sparse_indices));
	              		break;
	            	case TINYGLTF_COMPONENT_TYPE_SHORT:
	            	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
	              		index = (int)*(unsigned short *)
	              						(indices_buffer.data.data() +
	                                    indices_buffer_view.byteOffset +
	                                    accessor.sparse.indices.byteOffset +
	                                    (sparse_index * size_of_sparse_indices));
	              		break;
	            	case TINYGLTF_COMPONENT_TYPE_INT:
	            	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
	              		index = (int)*(unsigned int *)
	              						(indices_buffer.data.data() +
	                                  	indices_buffer_view.byteOffset +
	                                  	accessor.sparse.indices.byteOffset +
	                                  	(sparse_index * size_of_sparse_indices));
	              	break;
          		}
          		std::cout << "updating sparse data at index  : " << index << std::endl;
          		// index is now the target of the sparse index to patch in
          		const unsigned char *read_from = values_buffer.data.data() +
              									(values_buffer_view.byteOffset +
               									accessor.sparse.values.byteOffset) +
              									(sparse_index * (size_of_object_in_buffer * accessor.type));

		        /*
		        std::cout << ((float*)read_from)[0] << "\n";
		        std::cout << ((float*)read_from)[1] << "\n";
		        std::cout << ((float*)read_from)[2] << "\n";
		        */

          		unsigned char *write_to = tmp_buffer + index * (size_of_object_in_buffer * accessor.type);
          		memcpy(write_to, read_from, size_of_object_in_buffer * accessor.type);
        	}

	        // debug:
	        /*for(size_t p = 0; p < bufferView.byteLength/sizeof(float); p++)
	        {
	          float* b = (float*)tmp_buffer;
	          std::cout << "modified_buffer [" << p << "] = " << b[p] << '\n';
	        }*/

        	glBufferData(bufferView.target, bufferView.byteLength, tmp_buffer, GL_STATIC_DRAW);
        	delete[] tmp_buffer;
      	}
    	glBindBuffer(bufferView.target, 0);

    	gBufferState[i] = state;
    }
}


static void DrawMesh(tinygltf::Model &model, const tinygltf::Mesh &mesh) {
  	//// Skip curves primitive.
  	// if (gCurvesMesh.find(mesh.name) != gCurvesMesh.end()) {
  	//  return;
  	//}

  	// if (gGLProgramState.uniforms["diffuseTex"] >= 0) {
  	//  glUniform1i(gGLProgramState.uniforms["diffuseTex"], 0);  // TEXTURE0
  	//}

  	if(gGLProgramState.uniforms["isCurvesLoc"] >= 0)
    	glUniform1i(gGLProgramState.uniforms["isCurvesLoc"], 0);


  	for (size_t i = 0; i < mesh.primitives.size(); i++) {
    	const tinygltf::Primitive &primitive = mesh.primitives[i];

    	if(primitive.indices < 0) return;

    	// Assume TEXTURE_2D target for the texture object.
    	// glBindTexture(GL_TEXTURE_2D, gMeshState[mesh.name].diffuseTex[i]);

    	std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
    	std::map<std::string, int>::const_iterator itEnd(primitive.attributes.end());

    	for(; it != itEnd; it++){
      		assert(it->second >= 0);
      		const tinygltf::Accessor &accessor = model.accessors[it->second];

      		glBindBuffer(GL_ARRAY_BUFFER, gBufferState[accessor.bufferView].vb);
      			CheckErrors("bind buffer");
      			int size = 1;
      			if(accessor.type == TINYGLTF_TYPE_SCALAR) 
        			size = 1;
      			else if (accessor.type == TINYGLTF_TYPE_VEC2)
        			size = 2;
      			else if (accessor.type == TINYGLTF_TYPE_VEC3)
        			size = 3;
      			else if (accessor.type == TINYGLTF_TYPE_VEC4)
        			size = 4;
      			else 
        			assert(0);
      			
      			// it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
      			if ((it->first.compare("POSITION") == 0) || (it->first.compare("NORMAL") == 0) || (it->first.compare("TEXCOORD_0") == 0)) {
        			if(gGLProgramState.attribs[it->first] >= 0) {
          				// Compute byteStride from Accessor + BufferView combination.
          				int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
          				assert(byteStride != -1);
          				glVertexAttribPointer(gGLProgramState.attribs[it->first], size,
                                accessor.componentType,
                                accessor.normalized ? GL_TRUE : GL_FALSE,
                                byteStride, BUFFER_OFFSET(accessor.byteOffset));
          				CheckErrors("vertex attrib pointer");
          				glEnableVertexAttribArray(gGLProgramState.attribs[it->first]);
          				CheckErrors("enable vertex attrib array");
        			}
      			}
    	}

    	const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBufferState[indexAccessor.bufferView].vb);
    		CheckErrors("bind buffer");
    		int mode = -1;
    		if (primitive.mode == TINYGLTF_MODE_TRIANGLES)
      			mode = GL_TRIANGLES;
    		else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
      			mode = GL_TRIANGLE_STRIP;
    		else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN)
      			mode = GL_TRIANGLE_FAN;
    		else if (primitive.mode == TINYGLTF_MODE_POINTS)
      			mode = GL_POINTS;
    		else if (primitive.mode == TINYGLTF_MODE_LINE)
      			mode = GL_LINES;
    		else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP)
      			mode = GL_LINE_LOOP;
    		else
      			assert(0);
    		
    		glDrawElements(mode, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset));
    		CheckErrors("draw elements");

    		{
      			std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
      			std::map<std::string, int>::const_iterator itEnd(primitive.attributes.end());

      			for(; it != itEnd; it++)
        			if((it->first.compare("POSITION") == 0) || (it->first.compare("NORMAL") == 0) || (it->first.compare("TEXCOORD_0") == 0))
          				if(gGLProgramState.attribs[it->first] >= 0)
            				glDisableVertexAttribArray(gGLProgramState.attribs[it->first]);
    		}
  	}
}

// Hierarchically draw nodes
static void DrawNode(tinygltf::Model &model, const tinygltf::Node &node) {
  	// Apply xform

  	glPushMatrix();
  	if (node.matrix.size() == 16) {
    	// Use `matrix' attribute
    	glMultMatrixd(node.matrix.data());
  	} else {
    	// Assume Trans x Rotate x Scale order
    	if (node.scale.size() == 3)
      		glScaled(node.scale[0], node.scale[1], node.scale[2]);

    	if (node.rotation.size() == 4)
      		glRotated(node.rotation[0], node.rotation[1], node.rotation[2],node.rotation[3]);

    	if (node.translation.size() == 3)
      		glTranslated(node.translation[0], node.translation[1], node.translation[2]);
 
	}
	// std::cout << "node " << node.name << ", Meshes " << node.meshes.size() <<
  	// std::endl;

  	// std::cout << it->first << std::endl;
  	// FIXME(syoyo): Refactor.
  	// DrawCurves(scene, it->second);
  	if (node.mesh > -1) {
    	assert(node.mesh < model.meshes.size());
    	DrawMesh(model, model.meshes[node.mesh]);
  	}

  	// Draw child nodes.
  	for (size_t i = 0 ; i < node.children.size(); i++) {
    	assert(node.children[i] < model.nodes.size());
    	DrawNode(model, model.nodes[node.children[i]]);
  	}

  	glPopMatrix();
}

static void DrawModel(tinygltf::Model &model){
	#if 0
		std::map<std::string, tinygltf::Mesh>::const_iterator it(scene.meshes.begin());
		std::map<std::string, tinygltf::Mesh>::const_iterator itEnd(scene.meshes.end());

		for (; it != itEnd; it++) {
			DrawMesh(scene, it->second);
			DrawCurves(scene, it->second);
		}
	#else
	  	// If the glTF asset has at least one scene, and doesn't define a default one
	  	// just show the first one we can find
	  	assert(model.scenes.size() > 0);
	  	int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
	  	const tinygltf::Scene &scene = model.scenes[scene_to_display];
	  	for (size_t i = 0; i < scene.nodes.size(); i++)
	    	DrawNode(model, model.nodes[scene.nodes[i]]);

	#endif
}


bool LoadShader(GLenum shaderType,  // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER(or
                                    // maybe GL_COMPUTE_SHADER)
                GLuint &shader, const char *shaderSourceFilename) {
  	GLint val = 0;

  	// free old shader/program
  	if (shader != 0) {
    	glDeleteShader(shader);
  	}

  	std::vector<GLchar> srcbuf;
  	FILE *fp = fopen(shaderSourceFilename, "rb");
  	if (!fp) {
    	fprintf(stderr, "failed to load shader: %s\n", shaderSourceFilename);
    	return false;
  	}
  	fseek(fp, 0, SEEK_END);
  	size_t len = ftell(fp);
  	rewind(fp);
  	srcbuf.resize(len + 1);
  	len = fread(&srcbuf.at(0), 1, len, fp);
  	srcbuf[len] = 0;
  	fclose(fp);

  	const GLchar *srcs[1];
  	srcs[0] = &srcbuf.at(0);

  	shader = glCreateShader(shaderType);
  	glShaderSource(shader, 1, srcs, NULL);
  	glCompileShader(shader);
  	glGetShaderiv(shader, GL_COMPILE_STATUS, &val);

  	if (val != GL_TRUE) {
	    char log[4096];
	    GLsizei msglen;
	    glGetShaderInfoLog(shader, 4096, &msglen, log);
	    printf("%s\n", log);
	    // assert(val == GL_TRUE && "failed to compile shader");
	    printf("ERR: Failed to load or compile shader [ %s ]\n",
	        shaderSourceFilename);
	    return false;
  	}

  	printf("Load shader [ %s ] OK\n", shaderSourceFilename);
  	return true;
}

bool LinkShader(GLuint &prog, GLuint &vertShader, GLuint &fragShader) {
  	GLint val = 0;

  	if (prog != 0)
    	glDeleteProgram(prog);


  	prog = glCreateProgram();

  	glAttachShader(prog, vertShader);
  	glAttachShader(prog, fragShader);
  	glLinkProgram(prog);

  	glGetProgramiv(prog, GL_LINK_STATUS, &val);
  	assert(val == GL_TRUE && "failed to link shader");

  	printf("Link shader OK\n");

  	return true;
}



/*--------------------------------------------------------------------------*/


int main(int argc, char* argv[]){
	Model model;
	TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, argv[1]);
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

	if(!warn.empty())
	  	printf("Warn: %s\n", warn.c_str());

	if(!err.empty())
	  	printf("Err: %s\n", err.c_str());

	if(!ret) {
	  	printf("Failed to parse glTF\n");
	  	return -1;
	}

	printf("Loading complete\n");

	/*
	for(const auto &meshe : model.meshes) {
        for(const auto &primitive : meshe.primitives) {

            const auto &indicesAccessor = model.accessors[primitive.indices];

            std::unique_ptr<intArrayBase> indicesArrayPtr;
            const auto &buffer = mParent.model.buffers[bufferView.buffer];
			const auto dataAddress = buffer.data.data() + bufferView.byteOffset + indicesAccessor.byteOffset;
			const auto byteStride = indicesAccessor.ByteStride(bufferView);
			const auto count = indicesAccessor.count;

			switch (indicesAccessor.componentType){
		        case TINYGLTF_COMPONENT_TYPE_BYTE:
		            indicesArrayPtr = std::unique_ptr<intArray<char> >(new intArray<char>(arrayAdapter<char>(dataAddress, count, byteStride)));
		            break;

		        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		            indicesArrayPtr = std::unique_ptr<intArray<unsigned char> >(new intArray<unsigned char>(arrayAdapter<unsigned char>(dataAddress, count, byteStride)));
		            break;

		        case TINYGLTF_COMPONENT_TYPE_SHORT:
		            indicesArrayPtr = std::unique_ptr<intArray<short> >(new intArray<short>(arrayAdapter<short>(dataAddress, count, byteStride)));
		            break;

		        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		            indicesArrayPtr = std::unique_ptr<intArray<unsigned short> >(new intArray<unsigned short>(arrayAdapter<unsigned short>(dataAddress, count, byteStride)));
		            break;

		        case TINYGLTF_COMPONENT_TYPE_INT:
		            indicesArrayPtr = std::unique_ptr<intArray<int> >(new intArray<int>(arrayAdapter<int>(dataAddress, count, byteStride)));
		            break;

		        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		            indicesArrayPtr = std::unique_ptr<intArray<unsigned int> >(new intArray<unsigned int>(arrayAdapter<unsigned int>(dataAddress, count, byteStride)));
		            break;
		        default:
	            	break;
			}

			const auto &indices = *indicesArrayPtr;
			Mesh<float> loadedMesh;

			if (indicesArrayPtr){
        		// std::cout << "indices: ";
        		for (size_t i(0); i < indicesArrayPtr->size(); ++i) {
          			// std::cout << indices[i] << " ";
          			loadedMesh.faces.push_back(indices[i]);
        		}
        		// std::cout << '\n';
			}


			switch (accessor.type) {
				case TINYGLTF_TYPE_VEC3:
			        switch (accessor.componentType) {
			            case TINYGLTF_COMPONENT_TYPE_FLOAT:
			            	v3fArray positions(arrayAdapter<v3f>(dataPtr, count, byte_stride));
			            	for (size_t i{0}; i < positions.size(); ++i) {
	                			const auto v = positions[i];
	                			// std::cout << "positions[" << i << "]: (" << v.x << ", " << v.y << ", " << v.z << ")\n";

	                			loadedMesh.vertices.push_back(v.x * scale);
	                			loadedMesh.vertices.push_back(v.y * scale);
	                			loadedMesh.vertices.push_back(v.z * scale);
	            			}
			            	break;
			            case TINYGLTF_COMPONENT_TYPE_DOUBLE:
			            	v3dArray positions(arrayAdapter<v3d>(dataPtr, count, byte_stride));
			           		for (size_t i{0}; i < positions.size(); ++i) {
			                    const auto v = positions[i];
			                    // std::cout << "positions[" << i << "]: (" << v.x << ", " << v.y << ", " << v.z << ")\n";

			                    loadedMesh.vertices.push_back(v.x * scale);
			                    loadedMesh.vertices.push_back(v.y * scale);
			                    loadedMesh.vertices.push_back(v.z * scale);
			                }
			            	break;
			            default :
			            	break;
			        }
			    	break;
			    default:
			    	break;
			}

			for (const auto &attribute : primitive.attributes) {
				const auto attribAccessor = model.accessors[attribute.second];
				
				const auto &bufferView = model.bufferViews[attribAccessor.bufferView];
				const auto &buffer = model.buffers[bufferView.buffer];
				const auto dataPtr = buffer.data.data() + bufferView.byteOffset + attribAccessor.byteOffset;
				const auto byte_stride = attribAccessor.ByteStride(bufferView);
				const auto count = attribAccessor.count;

				if(attribute.first == "POSITION") {

				}
			 	if(attribute.first == "TEXCOORD_0") {

			 	}
			 	if(attribute.first == "NORMAL") {

			 	}
			}
        }
    }*/

	return 0 ;
}