//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MESH_H
#define PALADIN_MESH_H
#include "Asset.h"
#include "Vertex.h"

class MeshAsset: public Asset
{
public:
    MeshAsset(std::string n,std::vector<Paladin::Vertex> v, std::vector<std::uint32_t> i, AssetHandle<MaterialAsset> m) :name(n), vertices(std::move(v)),indices(std::move(i)),material(std::move(m)){
    }
    std::string name;
    std::vector<Paladin::Vertex> vertices;
    std::vector<std::uint32_t> indices;
    AssetHandle<MaterialAsset> material;
};

//Traits
template<>
struct UploadableToGPU<MeshAsset> {
    static constexpr bool value = true;
};

struct MeshRange {
    std::size_t start_v;
    std::size_t end_v;

    std::size_t start_i;
    std::size_t end_i;
};



#endif //PALADIN_MESH_H