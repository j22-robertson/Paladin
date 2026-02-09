//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MODEL_H
#define PALADIN_MODEL_H
#include "Asset.h"

struct ModelAsset : public Asset
{
public:
    std::vector<AssetHandle> meshes;
    std::vector<AssetHandle> materials;
};



#endif //PALADIN_MODEL_H