//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_TEXTURE2D_H
#define PALADIN_TEXTURE2D_H
#include "Asset.h"
#include "asset/AssetTraits.h"
class Texture2DAsset: public Asset
{
    struct Texture2D
    {
        std::uint32_t width =0;
        std::uint32_t height = 0;
    };

    Texture2D texture;
};

template<>
struct HasImporter<Texture2DAsset> {
    static constexpr bool value = true;
};

template<>
struct UploadableToGPU<Texture2DAsset> {
    static constexpr bool value = true;
};

#endif //PALADIN_TEXTURE2D_H