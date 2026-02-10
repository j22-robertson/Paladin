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
    PALADIN_SCOPED_CPU_PROFILE(file.c_str(), ProfileColors::Blue);
    std::filesystem::path root(PALADIN_ROOT_DIR);
    std::filesystem::path asset_directory = root / "assets";

    std::filesystem::path full_path{};

    //TODO: There is definitely a better way to handle this
    {
        PALADIN_SCOPED_CPU_PROFILE("Walk Directory", ProfileColors::Red);
        for (auto entry : std::filesystem::recursive_directory_iterator(asset_directory) ){
            if (entry.is_regular_file() && entry.path().filename() == file) {
                full_path = entry.path();
            }
        }
    }

   // PALADIN_LOG(INFO,"Loading Asset:"+file +" with full path: "+full_path.generic_string());
    int width, height, channels;
    unsigned char* data = nullptr;

    {
        PALADIN_SCOPED_CPU_PROFILE("stbi_load",ProfileColors::Green);
        data = stbi_load(full_path.generic_string().c_str(), &width,&height,&channels,4);
    }

    if (data != nullptr) {
        //TODO: This is probably not the best way of doing things.
        std::vector<unsigned char> pixels(width * height * channels);
        std::memcpy(pixels.data(),data, width*height*channels);

        stbi_image_free(data);

        return m_asset_registry->InsertAsset<Texture2DAsset>(std::make_unique<Texture2DAsset>(
            width,
            height,
            channels,pixels));


    }
    //stbi_image_free(data);
    return {};
}