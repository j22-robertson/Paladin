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
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class AssetRegistry;
class ModelImporter : public IAssetImporter<ModelAsset>
{
    public:
    ModelImporter(AssetRegistry& asset_registry) : IAssetImporter<ModelAsset>(asset_registry)
    {

    }

    AssetHandle<ModelAsset> LoadAsset(std::string file) override;
private:
    void ProcessNode(const aiNode* node, const aiScene* scene);
    private:
};

#endif //PALADIN_MODELIMPORTER_H