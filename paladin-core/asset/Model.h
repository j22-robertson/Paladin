//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MODEL_H
#define PALADIN_MODEL_H
#include "Asset.h"
#include "AssetTraits.h"

class MeshAsset;
class MaterialAsset;

struct ModelAsset : public Asset
{
public:
    std::vector<AssetHandle<MeshAsset>> meshes;
    std::vector<AssetHandle<MaterialAsset>> materials;
};

template<>
struct HasImporter<ModelAsset> {
    static constexpr bool value = true;
};



#endif //PALADIN_MODEL_H