//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MATERIAL_H
#define PALADIN_MATERIAL_H
#include <unordered_map>

#include "Asset.h"


enum MaterialProperty
{
    Albedo =0,
    Normal=1,
    Metallic=2,
    Roughness=3,
    AmbientOcclusion=4,
    Invalid = -1,
};


class MaterialAsset : public Asset
{
public:
    [[nodiscard]] AssetHandle GetTexture(const MaterialProperty property, AssetHandle texture) const
    {
        return textures.at(property);
    }
    void SetTexture(const MaterialProperty property, const AssetHandle texture)
    {
        textures.at(property) = texture;
    }
private:
    std::unordered_map<MaterialProperty, AssetHandle> textures;
};
#endif //PALADIN_MATERIAL_H