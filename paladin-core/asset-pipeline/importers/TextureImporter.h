//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_TEXTUREIMPORTER_H
#define PALADIN_TEXTUREIMPORTER_H
#include "Logger.h"
#include "asset/Texture2D.h"
#include "asset-pipeline/interfaces/IAssetImporter.h"

class TextureImporter : public IAssetImporter<Texture2DAsset>
{
public:
    TextureImporter() = delete;
    explicit TextureImporter(AssetRegistry& asset_registry);
    AssetHandle LoadAsset(std::string file) override;
};

inline TextureImporter::TextureImporter(AssetRegistry& asset_registry) : IAssetImporter<Texture2DAsset>(asset_registry)
{

}

inline AssetHandle TextureImporter::LoadAsset(std::string file)
{
    PALADIN_LOG(INFO,"File found:"+file);
    return {};
}
#endif //PALADIN_TEXTUREIMPORTER_H
