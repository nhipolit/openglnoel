// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

using namespace tinygltf;


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


	for(const auto &meshe : model.meshes) {
        for(const auto &primitive : meshe.primitives) {
        	/* VAO */
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
    }

	return 0 ;
}