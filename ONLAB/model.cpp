

#include "model.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>



namespace v {

    Model::Model(Device& device, const std::string MODEL_PATH, DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool)
         : device{ device }, MODEL_PATH{ MODEL_PATH }{

        loadModel(textlayout, normallayout, pool);
        createBuffers();

    }

    Model::~Model() {
        for (auto& t : textures) {
            t->destroy();
        }

        for( auto& part : meshparts){
            delete part.vertexBuffer;
            delete part.indexBuffer;
        }
    }


    void Model::draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, int currentframe, bool shadow, int set) {


        for (auto& part : meshparts) {

            VkBuffer buffers[] = { part.vertexBuffer->getBuffer()};
            VkDeviceSize offsets[] = { 0 };

            int id = part.matID;

            if (!shadow && textures[id]->getTextureDescriptorSets().size() != 0) {

                textures[id]->bindTexture(commandBuffer, layout, currentframe, set);
                
                textures[id]->bindNormalMap(commandBuffer, layout, currentframe, set +1);

            }
            else if (shadow && textures[id]->getTextureDescriptorSets().size() != 0 &&set != -1) {
                textures[id]->bindTexture(commandBuffer, layout, currentframe, set);
            }
            

            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, part.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(part.indices.size()), 1, 0, 0, 0);
        }


    }


    void Model::loadModel(DescriptorSetLayout& textlayout, DescriptorSetLayout& normallayout, DescriptorPool& pool) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(warn + err);
        }


        for (const auto& material : materials) {

            std::string opacityMap = "";
            material.alpha_texname != "" ? opacityMap = material.alpha_texname : opacityMap = "";

            textures.push_back(new Texture(device,textlayout, normallayout, pool, material.diffuse_texname, material.displacement_texname));

            // textures.emplace_back( device, swapChain,layout, pool, material.diffuse_texname,material.displacement_texname );
        }


        for (const auto& shape : shapes) {

            std::unordered_map<Vertex, uint32_t> uniqueVertices{};
            MeshPart meshpart = {};

            int i = 0;

            for (const auto& index : shape.mesh.indices) {


                Vertex vertex{};

                vertex.pos = {
                     attrib.vertices[3 * index.vertex_index + 0],
                     attrib.vertices[3 * index.vertex_index + 1],
                     attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.color = { 1.f, 1.f, 1.f };

                vertex.normal = {
                     attrib.normals[3 * index.normal_index + 0],
                     attrib.normals[3 * index.normal_index + 1],
                     attrib.normals[3 * index.normal_index + 2],
                };

                vertex.texCoord = {
                     attrib.texcoords[2 * index.texcoord_index + 0],
                     1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };



                vertex.tangent = {};
                vertex.bitangent = {};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(meshpart.vertices.size());
                    meshpart.vertices.push_back(vertex);
                }
                meshpart.indices.push_back(uniqueVertices[vertex]);

                i++;
                if (!(i % 3)) {

                    size_t indicesSize = meshpart.indices.size();
                    uint32_t i0 = meshpart.indices[indicesSize - 3];
                    uint32_t i1 = meshpart.indices[indicesSize - 2];
                    uint32_t i2 = meshpart.indices[indicesSize - 1];
                    Vertex& v0 = meshpart.vertices[i0];
                    Vertex& v1 = meshpart.vertices[i1];
                    Vertex& v2 = meshpart.vertices[i2];

                    glm::vec3 edge1 = v1.pos - v0.pos;
                    glm::vec3 edge2 = v2.pos - v0.pos;

                    glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
                    glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

                    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                    glm::vec3 T, B;
                    T.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    T.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    T.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                    B.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    B.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    B.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


                    v0.tangent = T;
                    v0.bitangent = B;
                    v1.tangent = T;
                    v1.bitangent = B;
                    v2.tangent = T;
                    v2.bitangent = B;
                }
            }
            for (int i = 0; i < meshpart.vertices.size(); i++) {
                meshpart.vertices[i].tangent = glm::normalize(meshpart.vertices[i].tangent);
                meshpart.vertices[i].bitangent = glm::normalize(meshpart.vertices[i].bitangent);
                glm::vec3& T = meshpart.vertices[i].tangent;
                glm::vec3& B = meshpart.vertices[i].bitangent;
                glm::vec3& N = meshpart.vertices[i].normal;
                T = glm::normalize(T - glm::dot(T, N) * N);
                B = glm::cross(N, T);
            }

            meshpart.matID = shape.mesh.material_ids[0]; 

            meshparts.push_back(meshpart);
        }
    }


    void Model::createBuffers() {
        for (auto& part : meshparts) {
            createVertexBuffer(part);
            createIndexBuffer(part);
        }
    }


    void Model::createVertexBuffer(MeshPart& meshpart) {

        VkDeviceSize bufferSize = sizeof(meshpart.vertices[0]) * meshpart.vertices.size();

        Buffer stagingBuffer = Buffer{ device, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)meshpart.vertices.data());

        meshpart.vertexBuffer = new Buffer(device, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        Helper::copyBuffer(device, stagingBuffer.getBuffer(), meshpart.vertexBuffer->getBuffer(), bufferSize);
    }


    void Model::createIndexBuffer(MeshPart& meshpart) {
        VkDeviceSize bufferSize = sizeof(meshpart.indices[0]) * meshpart.indices.size();

        Buffer stagingBuffer = Buffer{ device, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)meshpart.indices.data());

        meshpart.indexBuffer = new Buffer(device, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        Helper::copyBuffer(device, stagingBuffer.getBuffer(), meshpart.indexBuffer->getBuffer(), bufferSize);
    }
    








}