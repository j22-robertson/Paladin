//
// Created by jalr on 09-02-2026.
//
#include "ModelImporter.h"

#include <random>
#include <span>

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

    auto new_model_handle = m_asset_registry->InsertAsset<ModelAsset>();




    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(full_path.generic_string(), aiProcess_Triangulate|aiProcess_CalcTangentSpace|aiProcess_FlipUVs);
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
        m_asset_registry->AddDependency(material_handle,normal);
        m_asset_registry->AddDependency(material_handle,albedo);
        m_asset_registry->AddDependency(material_handle,metallic_roughness);
    }


    auto& node = scene->mRootNode;
    std::uint32_t mesh_count = 0;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE|| !node)
    {
        PALADIN_LOG(ERR, "ASSIMP:" +std::string(importer.GetErrorString()))
        return {};
    }
    mesh_count = scene->mNumMeshes;
    std::vector<std::uint32_t> mesh_to_mat;
    mesh_to_mat.resize(mesh_count);
    auto meshes = ProcessNode(node, scene,mesh_to_mat,materials,file);

    for (auto& mesh : meshes) {
        m_asset_registry->AddDependency(new_model_handle, mesh);
    }

    auto new_model = m_asset_registry->GetAsset<ModelAsset>(new_model_handle);

    new_model->materials = std::move(materials);
    new_model->meshes = std::move(meshes);
    new_model->mesh_to_material = std::move(mesh_to_mat);

    return new_model_handle;
}

std::vector<AssetHandle<MeshAsset>> ModelImporter::ProcessNode(const aiNode* node, const aiScene* scene,std::vector<std::uint32_t>& mesh_to_mat, const std::span<AssetHandle<MaterialAsset>> materials, const std::string& file)
{
    std::vector<AssetHandle<MeshAsset>> mesh_handles;

    for (int i = 0; i < node->mNumMeshes; i++) {
        std::vector<Paladin::Vertex> vertices{};

        std::vector<std::uint32_t> indices{};

        std::size_t mesh_index = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[mesh_index];

        vertices.resize(mesh->mNumVertices);




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
        if (mesh->HasVertexColors(0)) {
            for (int j = 0; j < mesh->mNumVertices; j++) {
                vertices[j].r = static_cast<float>(mesh->mColors[0][j].r);
                vertices[j].g = static_cast<float>(mesh->mColors[0][j].g);
                vertices[j].b = static_cast<float>(mesh->mColors[0][j].b);
            }
        }
        else
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(0,1);
            auto r_rand = dist(gen);
            auto b_rand = dist(gen);
            auto g_rand = dist(gen);
            for (int j = 0; j < mesh->mNumVertices; j++) {
                vertices[j].r =r_rand;
                vertices[j].g = b_rand;
                vertices[j].b = g_rand;
                vertices[j].a = 1.0;
            }
        }

        if (mesh->HasTextureCoords(0))
        {
            for (int j = 0; j < mesh->mNumVertices; j++)
            {
                vertices[j].u = static_cast<float>(mesh->mTextureCoords[0][j].x);
                vertices[j].v = static_cast<float>(mesh->mTextureCoords[0][j].y);
            }
        }

        for (int j =0; j < mesh->mNumFaces; j++) {
            const aiFace face = mesh->mFaces[j];
            for (int n = 0; n < face.mNumIndices; n++) {
                indices.push_back(face.mIndices[n]);
            }
        }
        std::string name = mesh->mName.Empty() ? file  + ":" +" Mesh:" +std::to_string(i) : mesh->mName.C_Str();
        auto mesh_handle = m_asset_registry->InsertAsset(std::make_unique<MeshAsset>(name,vertices,indices,materials[mesh->mMaterialIndex]));
        mesh_handles.push_back(mesh_handle);
        mesh_to_mat[mesh_index] = mesh->mMaterialIndex;
        m_asset_registry->AddDependency(mesh_handle, materials[mesh->mMaterialIndex]);
    }
    return mesh_handles;
}