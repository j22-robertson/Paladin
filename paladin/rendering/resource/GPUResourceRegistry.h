//
// Created by jalr on 26-02-2026.
//
#ifndef PALADIN_GPURESOURCEREGISTRY_H
#define PALADIN_GPURESOURCEREGISTRY_H

#include <map>
#include <wrl/client.h>

#include "D3D12MemAlloc.h"
#include "ResourceManager.h"
#include "asset/Asset.h"
#include "asset/AssetHandle.h"
#include "asset/Texture2D.h"
#include "asset/Model.h"
#include "asset/Material.h"
#include "asset/Mesh.h"
#include "asset-pipeline/interfaces/IAssetManager.h"
#include "utility/GenIndices.h"

struct GPUTexture2D : GPUResource
{
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation;
    UINT srv_descriptor_index = 0;
    UINT rtv_descriptor_index = 0;
};

struct GPUMesh: GPUResource
{
public:
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> allocation;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    std::uint32_t indices;
};

struct GPUMaterial: GPUResource
{
    GPUResourceHandle<GPUTexture2D> albedo;
    GPUResourceHandle<GPUTexture2D> normal;
    GPUResourceHandle<GPUTexture2D> roughness;
    GPUResourceHandle<GPUTexture2D> metallic;
};

struct GPUModel : GPUResource
{
    std::vector<GPUResourceHandle<GPUMesh>> mesh_handles;
    std::vector<GPUResourceHandle<GPUMaterial>> material_handles;
    std::vector<std::uint32_t> mesh_to_material;
};

class GPUResourceRegistry {
    std::unordered_map<std::size_t, std::unique_ptr<ResourceManager>> managers;


    GPUResourceRegistry()
    {
        managers.insert({typeid(Texture2DAsset).hash_code(),std::make_unique<IResourceManager<GPUTexture2D>>()});
        managers.insert({typeid(MeshAsset).hash_code(),std::make_unique<IResourceManager<GPUMesh>>()});
        managers.insert({typeid(ModelAsset).hash_code(),std::make_unique<IResourceManager<GPUModel>>()});
        managers.insert({typeid(MaterialAsset).hash_code(),std::make_unique<IResourceManager<GPUMaterial>>()});
    }
    template<typename T>
        requires IsPaladinResource<T>
    T* Get(OpaqueAssetHandle handle)
    {
        if (IResourceManager<T>* manager = GetManager<T>(); manager!=nullptr)
        {
            return manager->Get(handle);
        }
        return nullptr;
    }
    template<typename T>
    requires IsPaladinResource<T>
    GPUResourceHandle<T> Insert(std::unique_ptr<T> resource, OpaqueAssetHandle handle)
    {
        if (IResourceManager<T>* manager = GetManager<T>(); manager!=nullptr)
        {
            return manager->Insert(std::move(resource),handle);
        }
        return {};
    }


    template<typename T> requires IsPaladinResource<T>
    IResourceManager<T>* GetManager(std::size_t hash)
    {
        if (auto it = managers.find(hash); it!=managers.end()) {
            return static_cast<IResourceManager<T>*>(it->second.get());
        }
        return nullptr;
    }

};


#endif //PALADIN_GPURESOURCEREGISTRY_H