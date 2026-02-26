//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MODELIMPORTER_H
#define PALADIN_MODELIMPORTER_H
#include "Logger.h"
#include "asset/AssetHandle.h"
#include "asset/Model.h"
#include "asset-pipeline/interfaces/IAssetImporter.h"
#include "asset/Material.h"
#include "asset/Mesh.h"
#include <filesystem>
#include <span>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "profiling/Profiling.h"

class AssetRegistry;
class ModelImporter : public IAssetImporter<ModelAsset>
{
    public:
    ModelImporter(AssetRegistry& asset_registry) : IAssetImporter<ModelAsset>(asset_registry)
    {

    }

    AssetHandle<ModelAsset> LoadAsset(std::string file) override;
private:
    std::vector<AssetHandle<MeshAsset>> ProcessNode(const aiNode* node, const aiScene* scene, std::vector<std::uint32_t>& mesh_to_mat, std::span<AssetHandle<MaterialAsset>> materials, const std::string& file);
    private:
};

#endif //PALADIN_MODELIMPORTER_H