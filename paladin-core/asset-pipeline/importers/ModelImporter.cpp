//
// Created by jalr on 09-02-2026.
//
#include "ModelImporter.h"
#include "asset-pipeline/AssetRegistry.h"
AssetHandle ModelImporter::LoadAsset(std::string file)
{
    PALADIN_LOG(INFO, "Loading model asset" + file)
    m_asset_registry->LoadAsset<Texture2DAsset>("Testing");

    return {};
}