//
// Created by jalr on 09-02-2026.
//
#include "ModelImporter.h"

#include "Vertex.h"
#include "../../../cmake-build-debug/_deps/glm-src/glm/vec3.hpp"
#include "asset-pipeline/AssetRegistry.h"


AssetHandle<ModelAsset> ModelImporter::LoadAsset(std::string file)
{
    std::vector<AssetHandle<MeshAsset>> loaded_meshes;
    Assimp::Importer importer;
    PALADIN_LOG(INFO, "LOADING ASSET:"+file)

    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate|aiProcess_CalcTangentSpace);
    if (!scene) return {};
    std::vector<AssetHandle<MaterialAsset>> materials;

    std::unordered_map<std::uint32_t, AssetHandle<Texture2DAsset>> material_map;
    for (int i = 0; i < scene->mNumMaterials; i++)
    {
        MaterialAsset material_asset;

        aiMaterial* material = scene->mMaterials[i];
        aiString path{};
        material->GetTexture(aiTextureType_BASE_COLOR, 0 , &path);
        
    }

    auto& node = scene->mRootNode;
    std::uint32_t mesh_count = 0;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE|| !node)
    {
        PALADIN_LOG(ERR, "ASSIMP:" +std::string(importer.GetErrorString()))
        return {};
    }



    PALADIN_LOG(INFO, "Loading model asset" + file)
    //m_asset_registry->LoadAsset<Texture2DAsset>("Testing");

    return {};
}

void ModelImporter::ProcessNode(const aiNode* node, const aiScene* scene)
{
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        std::vector<Paladin::Vertex> vertices{};

        std::vector<std::uint32_t> indices{};

        std::size_t mesh_index = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[mesh_index];

        vertices.resize(mesh->mNumVertices);

        indices.resize(mesh->mNumFaces*3);
        

        for (int j = 0; j < mesh->mNumVertices; j++)
        {
            vertices[j].x = static_cast<float>(mesh->mVertices[j].x);
            vertices[j].y = static_cast<float>(mesh->mVertices[j].y);
            vertices[j].z = static_cast<float>(mesh->mVertices[j].z);
        }
        if (mesh->HasNormals())
        {
            for (int j = 0; j < mesh->mNumVertices; j++)
            {
                vertices[j].nx = static_cast<float>(mesh->mNormals[j].x);
                vertices[j].ny = static_cast<float>(mesh->mNormals[j].y);
                vertices[j].nz = static_cast<float>(mesh->mNormals[j].z);
            }
        }
        if (mesh->HasTangentsAndBitangents())
        {
            for (int j = 0; j < mesh->mNumVertices; j++)
            {
                vertices[j].tx = static_cast<float>(mesh->mTangents[j].x);
                vertices[j].ty = static_cast<float>(mesh->mTangents[j].y);
                vertices[j].tz = static_cast<float>(mesh->mTangents[j].z);

                vertices[j].btx = static_cast<float>(mesh->mBitangents[j].x);
                vertices[j].bty = static_cast<float>(mesh->mBitangents[j].y);
                vertices[j].btz = static_cast<float>(mesh->mBitangents[j].z);
            }
        }



    }
}