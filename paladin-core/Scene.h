//
// Created by James Robertson on 02/03/2026.
//

#ifndef PALADIN_SCENE_H
#define PALADIN_SCENE_H
#include <map>
#include <unordered_map>
#include <vector>

#include "Camera.h"
#include "Transform.h"
#include "asset/AssetHandle.h"

struct ModelAsset;


struct Scene {
    struct SceneEntry {
        AssetHandle<ModelAsset> model_handle;
        std::uint32_t transform_index;
        std::uint32_t transform_count;
    };
    std::vector<Transform> all_transforms;
    std::vector<SceneEntry> model_entries;
    std::vector<AssetHandle<ModelAsset>> all_models;
    Camera camera;
};


struct RenderFrameData {
    struct ModelBatch {
        OpaqueAssetHandle handle;
        std::vector<TransformData> transforms;
    };
    void insert(AssetHandle<ModelAsset>  model,Transform& transform);
    //void insertAABB(Transform& transform);
    std::vector<ModelBatch> batches;
    std::vector<TransformData> debug_aabb_transforms;
    std::unordered_map<std::uint32_t, std::uint32_t> id_to_batch;
    std::uint32_t instance_buffer_id = 0;
    std::uint32_t frame_index = 0;
    std::uint32_t aligned_bytes_per_buffer = 0;
    Camera camera;
};

struct FrameUploadData {
    struct MeshDrawBatch {
        OpaqueAssetHandle mesh_handle;
        std::uint32_t instance_index; // Offset to where transforms start for instances of this mesh
        std::uint32_t instance_count; // Number of transforms associated with mesh (Draw count)
    };
    std::vector<TransformData> transforms;
    std::vector<MeshDrawBatch> visible_meshes;

    std::vector<std::uint32_t> instance_indices;
    std::uint32_t instance_buffer_id = 0;
    std::uint32_t frame_index = 0;
    std::uint32_t aligned_bytes_per_buffer = 0;
    std::uint32_t id_aligned_bytes = 0;
    std::uint32_t id_heap_index = 0;
    Camera camera;
};

inline void RenderFrameData::insert(AssetHandle<ModelAsset> model, Transform &transform) {
    if (id_to_batch.contains(model.inner.index)) {
        batches[id_to_batch[model.inner.index]].transforms.push_back(transform.GetData());
        return;
    }
    ModelBatch batch{};
    batch.handle = model;
    batch.transforms.push_back(transform.GetData());
    batches.push_back(batch);
    id_to_batch.insert(std::make_pair(model.inner.index, batches.size()-1));
}


#endif //PALADIN_SCENE_H
