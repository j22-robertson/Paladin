//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MESH_H
#define PALADIN_MESH_H
#include "Asset.h"
#include "Vertex.h"

class MeshAsset: public Asset
{
    std::vector<Paladin::Vertex> vertices;
    std::vector<std::uint32_t> indices;
    AssetHandle<MaterialAsset> material;
};

//Traits
template<>
struct UploadableToGPU<MeshAsset> {
    static constexpr bool value = true;
};
#endif //PALADIN_MESH_H