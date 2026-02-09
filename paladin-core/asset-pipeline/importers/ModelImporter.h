//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MODELIMPORTER_H
#define PALADIN_MODELIMPORTER_H
#include "Logger.h"

#include "asset/Model.h"
#include "asset-pipeline/interfaces/IAssetImporter.h"
class AssetRegistry;
class ModelImporter : public IAssetImporter<ModelAsset>
{
    public:
    inline ModelImporter(AssetRegistry& asset_registry) : IAssetImporter<ModelAsset>(asset_registry)
    {

    }

    AssetHandle LoadAsset(std::string file) override;
    private:
};

#endif //PALADIN_MODELIMPORTER_H