//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_IASSETMANAGER_H
#define PALADIN_IASSETMANAGER_H
#include "asset/Asset.h"
#include <unordered_map>
#include <unordered_set>
#include "IAssetManagerBase.h"
#include "profiling/Profiling.h"
#include <span>

// TODO: Maybe Optional is better here?
template<typename T>
requires IsPaladinAsset<T>
class IAssetManager : public IAssetManagerBase
{
public:
    T* GetAsset(AssetHandle<T> handle) {
        if (asset_index_allocator.IsValid(handle.inner)) {
            return asset_storage[handle.inner.index].get();
        }
        return nullptr;
    }


    const T* GetAsset(AssetHandle<T> handle) const{
        if (asset_index_allocator.IsValid(handle.inner)) {
            return asset_storage[handle.inner.index].get();
        }
        return nullptr;
    }

    std::vector<AssetHandle<T>> InsertAssets(std::vector<std::unique_ptr<T>> assets) {
        std::vector<AssetHandle<T>> new_asset_handles;
        if (assets.empty()) return {};

        std::uint32_t greatest_index = 0;
        for (auto& asset : assets) {
            if (asset == nullptr)[[unlikely]] {
                PALADIN_LOG(ERR, "Inserted asset of type:"+std::string(typeid(T).name())+" is null")
                return {};
            }
            auto new_handle = asset_index_allocator.Allocate();
            if (greatest_index< new_handle.index) {
                greatest_index = new_handle.index;
            }
            new_asset_handles.push_back({new_handle});
        }

        if (asset_storage.size() <= greatest_index) {
            asset_storage.resize(greatest_index + 1);
        }
        for (std::size_t i = 0; i < assets.size(); i++) {
            asset_storage[new_asset_handles[i].inner.index] = std::move(assets[i]);
        }
        return new_asset_handles;
    }

    AssetHandle<T> InsertAsset() {
        PALADIN_SCOPED_CPU_PROFILE(std::string("Insert:"+std::string(typeid(T).name())).c_str(), ProfileColors::Green);
        auto new_index = asset_index_allocator.Allocate();
        if (asset_storage.size()<=new_index.index) {
            asset_storage.resize(new_index.index+1);
            asset_storage[new_index.index] = std::make_unique<T>();
        }
        asset_storage[new_index.index] = std::make_unique<T>();
        return {new_index};
    };

    AssetHandle<T> InsertAsset(std::unique_ptr<T> asset) {
        PALADIN_SCOPED_CPU_PROFILE(std::string("Insert:"+std::string(typeid(T).name())).c_str(), ProfileColors::Green);
        if (asset == nullptr) {
            PALADIN_LOG(ERR, "Inserted asset of type:"+std::string(typeid(T).name())+" is null")
            return {};
        }
        auto new_index = asset_index_allocator.Allocate();
        if (asset_storage.size()<=new_index.index) {
            asset_storage.resize(new_index.index+1);
        }
        asset_storage[new_index.index] = std::move(asset);
        return {new_index};
    };
    void RemoveAsset(AssetHandle<T> handle) {
        if (asset_index_allocator.IsValid(handle.inner)) {
            asset_storage[handle.inner.index].reset();
            asset_index_allocator.Deallocate(handle.inner);
        }
    }
    void RemoveOpaque(OpaqueAssetHandle handle) override {
        AssetHandle<T> typed_handle = AssetHandle<T>{.inner = handle.inner};
        RemoveAsset(typed_handle);
    }
    bool IsValid(AssetHandle<T>& handle) {
        return asset_index_allocator.IsValid(handle.inner);
    }
private:
    bool lazy_load = false;
    GenerationalIndexAllocator asset_index_allocator;
    std::vector<std::unique_ptr<T>> asset_storage;


    std::unordered_set<std::uint32_t> to_load;
    std::unordered_set<std::uint32_t> registered;
};

#endif //PALADIN_IASSETMANAGER_H