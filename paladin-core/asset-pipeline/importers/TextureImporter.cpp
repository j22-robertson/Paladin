//
// Created by James Robertson on 10/02/2026.
//
#include "asset-pipeline/importers/TextureImporter.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "asset-pipeline/AssetRegistry.h"


TextureImporter::TextureImporter(AssetRegistry& asset_registry) : IAssetImporter<Texture2DAsset>(asset_registry)
{

}

AssetHandle<Texture2DAsset> TextureImporter::LoadAsset(std::string file)
{
    std::filesystem::path root(PALADIN_ROOT_DIR);
    std::filesystem::path asset_directory = root / "assets";

    std::filesystem::path full_path{};

    //TODO: There is definitely a better way to handle this
    for (auto entry : std::filesystem::recursive_directory_iterator(asset_directory) ){
        if (entry.is_regular_file() && entry.path().filename() == file) {
            full_path = entry.path();
        }
    }
    PALADIN_LOG(INFO,"Loading Asset:"+file +" with full path: "+full_path.generic_string());
    int width, height, channels;
    unsigned char* data = stbi_load(full_path.generic_string().c_str(), &width,&height,&channels,4);
    if (data != nullptr) {
        PALADIN_LOG(INFO, "Succesfully loaded image!")

        return m_asset_registry->InsertAsset<Texture2DAsset>(std::make_unique<Texture2DAsset>());
    }
    delete data;
    data = nullptr;
    return {};
}