//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_MESH_H
#define PALADIN_MESH_H
#include <utility>
#include "Asset.h"
#include "AssetTraits.h"
#include "AssetHandle.h"
#include "Material.h"
#include "Vertex.h"
#include "glm/vec3.hpp"
#include "utility/BoundingBox.h"




class MeshAsset: public Asset
{
public:
    MeshAsset(std::string n,std::vector<Paladin::Vertex> v,
        std::vector<std::uint32_t> i,
        const AssetHandle<MaterialAsset> m, const AABB& aabb) :
    name(std::move(n)),
    vertices(std::move(v)),
    indices(std::move(i)),
    material(m),
    bounding_box(aabb)
    {}
    std::string name;
    std::vector<Paladin::Vertex> vertices;
    std::vector<std::uint32_t> indices;
    AssetHandle<MaterialAsset> material;
    AABB bounding_box;
};

//Traits
template<>
struct UploadableToGPU<MeshAsset> {
    static constexpr bool value = true;
};

struct MeshRange {
    // The offset
    std::uint32_t vertex_start_location = 0;
    std::uint32_t vertex_count = 0;
    //Off set
    std::uint32_t index_start_location = 0;
    std::uint32_t index_count = 0;
};



#endif //PALADIN_MESH_H