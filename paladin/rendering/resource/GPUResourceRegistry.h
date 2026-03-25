//
// Created by jalr on 26-02-2026.
//
#ifndef PALADIN_GPURESOURCEREGISTRY_H
#define PALADIN_GPURESOURCEREGISTRY_H

#include <map>
#include <wrl/client.h>

#include "D3D12MemAlloc.h"
#include "ResourceManager.h"
#include "../descriptor/DescriptorHeapManager.h"
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
    ViewHandle srv_handle;
    ViewHandle rtv_handle;
};

struct GPUMesh: GPUResource
{
public:
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> vertex_allocation;
    Microsoft::WRL::ComPtr<D3D12MA::Allocation> index_allocation;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    std::uint32_t indices;
    OpaqueAssetHandle material;
    AABB aabb;
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
    std::map<std::size_t, std::size_t> asset_to_resource_type;

public:
    GPUResourceRegistry()
    {
        managers.insert({typeid(GPUTexture2D).hash_code(),std::make_unique<IResourceManager<GPUTexture2D>>()});
        managers.insert({typeid(GPUMesh).hash_code(),std::make_unique<IResourceManager<GPUMesh>>()});
        managers.insert({typeid(GPUModel).hash_code(),std::make_unique<IResourceManager<GPUModel>>()});
        managers.insert({typeid(GPUMaterial).hash_code(),std::make_unique<IResourceManager<GPUMaterial>>()});

        asset_to_resource_type.insert({typeid(MaterialAsset).hash_code(),typeid(GPUMaterial).hash_code()});
        asset_to_resource_type.insert({typeid(Texture2DAsset).hash_code(),typeid(GPUTexture2D).hash_code()});
        asset_to_resource_type.insert({typeid(ModelAsset).hash_code(),typeid(GPUModel).hash_code()});
        asset_to_resource_type.insert({typeid(MeshAsset).hash_code(),typeid(GPUMesh).hash_code()});
    }
    template<typename T>
        requires IsPaladinResource<T>
    T* Get(GPUResourceHandle<T> handle)
    {
        if (IResourceManager<T>* manager = GetManager<T>(); manager!=nullptr)
        {
            return manager->Get(handle);
        }
        return nullptr;
    }

    template<typename T>
        requires IsPaladinResource<T>
    T* GetOpaque(OpaqueAssetHandle handle)
    {
        if (IResourceManager<T>* manager = GetManager<T>(handle.type_id); manager!=nullptr)
        {
            return manager->GetOpaque(handle);
        }
        return nullptr;
    }
    template<typename T>
        requires IsPaladinResource<T>
    bool IsValid(OpaqueAssetHandle handle)
    {
        if (auto it = managers.find(asset_to_resource_type[handle.type_id]); it!=managers.end()) {
            return it->second->IsValid(handle.inner);
        }
        return false;
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

    void Clear()
    {
        managers.clear();
    }

    template<typename T> requires IsPaladinResource<T>
    IResourceManager<T>* GetManager(std::size_t hash)
    {
        if (auto it = managers.find(asset_to_resource_type[hash]); it!=managers.end()) {
            return static_cast<IResourceManager<T>*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T> requires IsPaladinResource<T>
    IResourceManager<T>* GetManager()
    {
        if (auto it = managers.find(typeid(T).hash_code()); it!=managers.end()) {
            return static_cast<IResourceManager<T>*>(it->second.get());
        }
        return nullptr;
    }

};


#endif //PALADIN_GPURESOURCEREGISTRY_H