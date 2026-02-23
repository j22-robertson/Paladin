//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MATERIAL_H
#define PALADIN_MATERIAL_H
#include <unordered_map>
#include "AssetHandle.h"
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

class Texture2DAsset;

class MaterialAsset : public Asset
{
public:
    void SetName(std::string new_name) {
        name = new_name;
    }
    const std::string& GetName() {
        return name;
    }
    [[nodiscard]] AssetHandle<Texture2DAsset> GetTexture(MaterialProperty property)
    {
        return textures.at(property);
    }
    void SetTexture(const MaterialProperty property, const AssetHandle<Texture2DAsset> texture)
    {
        textures.insert({property, texture});
    }
private:
    std::unordered_map<MaterialProperty, AssetHandle<Texture2DAsset>> textures;
    std::string name;
};
#endif //PALADIN_MATERIAL_H