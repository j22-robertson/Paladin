//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_TEXTURE2D_H
#define PALADIN_TEXTURE2D_H
#include "Asset.h"
#include "asset/AssetTraits.h"
class Texture2DAsset: public Asset
{
public:
    Texture2DAsset(std::uint32_t w,std::uint32_t h,std::uint32_t c,std::unique_ptr<unsigned char> p): width(w), height(h), channels(c), pixel_data(std::move(p)) {
    }
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t channels = 0;
    std::unique_ptr<unsigned char> pixel_data;
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