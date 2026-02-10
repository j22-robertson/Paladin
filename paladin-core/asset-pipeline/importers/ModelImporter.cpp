//
// Created by jalr on 09-02-2026.
//
#include "ModelImporter.h"

#include "Vertex.h"
#include "../../../cmake-build-debug/_deps/glm-src/glm/vec3.hpp"
#include "asset-pipeline/AssetRegistry.h"
#include "profiling/Profiling.h"


AssetHandle<ModelAsset> ModelImporter::LoadAsset(std::string file)
{
    std::filesystem::path root(PALADIN_ROOT_DIR);
    std::filesystem::path asset_directory = root / "assets";

    std::filesystem::path full_path{};
    //TODO: This is probably so inefficient.
    for (auto entry : std::filesystem::recursive_directory_iterator(asset_directory) ){
        if (entry.is_regular_file() && entry.path().filename() == file) {
            full_path = entry.path();
        }
    }


    std::unique_ptr<ModelAsset> model = std::make_unique<ModelAsset>();
    std::vector<AssetHandle<MeshAsset>> loaded_meshes;

    Assimp::Importer importer;
  //  PALADIN_LOG(INFO, "LOADING ASSET:"+file)

    const aiScene* scene = importer.ReadFile(full_path.generic_string(), aiProcess_Triangulate|aiProcess_CalcTangentSpace);
    if (!scene) return {};
    std::vector<AssetHandle<MaterialAsset>> materials = std::vector<AssetHandle<MaterialAsset>>();

    for (int i = 0; i < scene->mNumMaterials; i++)
    {
        std::string mat_name = file+" Material:"+std::to_string(i);


        PALADIN_SCOPED_CPU_PROFILE(mat_name.c_str(), ProfileColors::Red);
        auto new_material = std::make_unique<MaterialAsset>();

        aiMaterial* material = scene->mMaterials[i];
        if (material == nullptr) continue;
        aiString path{};
        material->GetTexture(aiTextureType_BASE_COLOR, 0 , &path);
        if (path.Empty()) continue;
        auto albedo = m_asset_registry->ImportAsset<Texture2DAsset>(path.C_Str());
        aiString metallic_roughness_path {};


        material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &metallic_roughness_path);
        auto metallic_roughness = m_asset_registry->ImportAsset<Texture2DAsset>(metallic_roughness_path.C_Str());

        aiString normal_path{};
        material->GetTexture(aiTextureType_NORMALS, 0, &normal_path);
        auto normal = m_asset_registry->ImportAsset<Texture2DAsset>(normal_path.C_Str());

        new_material->SetName(file+" Material:"+std::to_string(i));
        new_material->SetTexture(Albedo, albedo);
        new_material->SetTexture(Roughness, metallic_roughness);
        new_material->SetTexture(Metallic, metallic_roughness);
        new_material->SetTexture(Normal, normal);
        auto material_handle = m_asset_registry->InsertAsset<MaterialAsset>(std::move(new_material));
        materials.push_back(material_handle);

    }

    auto& node = scene->mRootNode;
    std::uint32_t mesh_count = 0;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE|| !node)
    {
        PALADIN_LOG(ERR, "ASSIMP:" +std::string(importer.GetErrorString()))
        return {};
    }

    model->materials = std::move(materials);


    return m_asset_registry->InsertAsset(std::move(model));
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