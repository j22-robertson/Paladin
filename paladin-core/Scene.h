//
// Created by James Robertson on 02/03/2026.
//

#ifndef PALADIN_SCENE_H
#define PALADIN_SCENE_H
#include <map>
#include <unordered_map>
#include <vector>

#include "Transform.h"
#include "asset/AssetHandle.h"

struct ModelAsset;

struct RenderFrameData {
    struct ModelBatch {
        OpaqueAssetHandle handle;
        std::vector<TransformData> transforms;
    };
    void insert(AssetHandle<ModelAsset>  model,Transform& transform);
    std::vector<ModelBatch> batches;
    std::unordered_map<std::uint32_t, std::uint32_t> id_to_batch;
    std::uint32_t instance_buffer_id = 0;
    std::uint32_t frame_index = 0;
    std::uint32_t aligned_bytes_per_buffer = 0;
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
