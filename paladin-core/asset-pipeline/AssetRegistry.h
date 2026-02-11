//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_ASSETREGISTRY_H
#define PALADIN_ASSETREGISTRY_H
#include <map>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include "asset/Texture2D.h"
#include "importers/ModelImporter.h"
#include "interfaces/IAssetManager.h"
#include "interfaces/IAssetImporter.h"
#include "importers/TextureImporter.h"
#include "asset/AssetTraits.h"

using AssetManager = std::unique_ptr<IAssetManagerBase>;


using AssetImporter = std::unique_ptr<IAssetImporterBase>;

///TODO: Maybe RequestImport(x) returning a handle that will be valid in the future, puts task on queue for multithreading?
/// MORE ON THIS: create handles upfront for assets we know will be loaded. maybe add a CommitAsset() for locking when writing
/// Create a queue and thread pool or maybe use async with future to send tasks off. When completed remove from the not_loaded set (set of OpaqueAssetHandle)
/// Since only some assets need to be imported we can easily construct other assets e.g. Model/Scene, Material
class AssetRegistry {
public:
    AssetRegistry()
    {
        asset_importers.insert({typeid(Texture2DAsset).hash_code(),std::make_unique<TextureImporter>(*this)});
        asset_importers.insert({typeid(ModelAsset).hash_code(),std::make_unique<ModelImporter>(*this)});

        asset_managers.insert({typeid(Texture2DAsset).hash_code(),std::make_unique<IAssetManager<Texture2DAsset>>()});
        asset_managers.insert({typeid(ModelAsset).hash_code(),std::make_unique<IAssetManager<Texture2DAsset>>()});
        asset_managers.insert({typeid(MaterialAsset).hash_code(),std::make_unique<IAssetManager<Texture2DAsset>>()});
        asset_managers.insert({typeid(MeshAsset).hash_code(),std::make_unique<IAssetManager<Texture2DAsset>>()});

    }


    template<typename T> requires IsPaladinAsset<T>
    T* GetAsset(AssetHandle<T> handle);

    template<typename T> requires IsImportable<T>
    AssetHandle<T> ImportAsset(std::string file);

    // Insert multiple assets
    template<typename T> requires IsPaladinAsset<T>
    std::vector<AssetHandle<T>> InsertAssets(std::vector<std::unique_ptr<T>> assets);

    // Insert a single asset
    template<typename T> requires IsPaladinAsset<T>
    AssetHandle<T> InsertAsset(std::unique_ptr<T> asset);

    // TODO: Maybe rename. Insert a null asset for later writing
    template<typename T> requires IsPaladinAsset<T>
    AssetHandle<T> InsertAsset();

    template<typename T> requires IsPaladinAsset<T>
    bool IsValid(AssetHandle<T> handle);

    template<typename T> requires IsPaladinAsset<T>
    void Remove(AssetHandle<T> handle);

    void AddDependency(OpaqueAssetHandle parent, OpaqueAssetHandle child);
    void AddDependencies(OpaqueAssetHandle parent, const std::vector<OpaqueAssetHandle> &children);

    template<typename T> requires IsImportable<T>
    void RequestImport(const std::string& file);

private:

    template<typename T> requires IsImportable<T>
    IAssetImporter<T>* GetImporter();
    template<typename T> requires IsPaladinAsset<T>
    IAssetManager<T>* GetManager();

    void RemoveOpaque(OpaqueAssetHandle opaque_handle);

    std::unordered_map<std::size_t, AssetManager> asset_managers {};
    std::unordered_map<std::size_t, AssetImporter> asset_importers{};

    std::map<OpaqueAssetHandle, std::vector<OpaqueAssetHandle>> dependencies;
    std::map<OpaqueAssetHandle, std::set<OpaqueAssetHandle>> references;
};

inline void AssetRegistry::AddDependency(OpaqueAssetHandle parent, OpaqueAssetHandle child) {
    dependencies[parent].push_back(child);
    references[child].insert(parent);
    return;
}

inline void AssetRegistry::AddDependencies(const OpaqueAssetHandle parent, const std::vector<OpaqueAssetHandle> &children) {
    for (const auto child : children) {
        AddDependency(parent, child);
    }
}


inline void AssetRegistry::RemoveOpaque(OpaqueAssetHandle opaque_handle) {
    if (!dependencies[opaque_handle].empty()) {
        for (OpaqueAssetHandle& dependency : dependencies[opaque_handle]) {
            references[dependency].erase(opaque_handle);
            if (references[dependency].empty()) {
                RemoveOpaque(dependency);
            }
        }
    }
    dependencies[opaque_handle].clear();
    if (const auto it = asset_managers.find(opaque_handle.type_id); it->second!=nullptr) {
        it->second->RemoveOpaque(opaque_handle);
    }
}


template<typename T> requires IsPaladinAsset<T>
T* AssetRegistry::GetAsset(AssetHandle<T> handle) {
    if (auto manager = GetManager<T>(); manager != nullptr) {
        return manager->GetAsset(handle);
    }
    return nullptr;
}
template<typename T> requires IsImportable<T>
AssetHandle<T> AssetRegistry::ImportAsset(std::string file)
{
    PALADIN_SCOPED_CPU_PROFILE("ImportAsset", ProfileColors::Green);
    if (auto importer = GetImporter<T>(); importer!= nullptr)[[likely]] {
        return importer->LoadAsset(file);
    }
    std::string type_name = typeid(T).name();
    PALADIN_LOG(ERR, "Unable to find Importer for Type:"+type_name)
    return{};
}



template<typename T> requires IsImportable<T>
IAssetImporter<T>* AssetRegistry::GetImporter() {
    if (auto it = asset_importers.find(typeid(T).hash_code()); it!=asset_importers.end()) {
        return static_cast<IAssetImporter<T>*>(it->second.get());
    }
    return nullptr;
}
template<typename T> requires IsPaladinAsset<T>
std::vector<AssetHandle<T>> AssetRegistry::InsertAssets(std::vector<std::unique_ptr<T>> assets) {

    if (auto manager = GetManager<T>(); manager != nullptr) {
        return manager->InsertAssets(assets);
    }
    return {};
}


template<typename T> requires IsPaladinAsset<T>
AssetHandle<T> AssetRegistry::InsertAsset() {
    PALADIN_SCOPED_CPU_PROFILE("InsertAsset", ProfileColors::Green);
    if (auto manager = GetManager<T>(); manager != nullptr) {
        return manager->InsertAsset();
    }
    return {};
}

template<typename T> requires IsPaladinAsset<T>
AssetHandle<T> AssetRegistry::InsertAsset(std::unique_ptr<T> asset) {
    PALADIN_SCOPED_CPU_PROFILE("InsertAsset", ProfileColors::Green);
    if (auto manager = GetManager<T>(); manager != nullptr) {
        return manager->InsertAsset(std::move(asset));
    }
    return {};
}

template<typename T> requires IsPaladinAsset<T>
bool AssetRegistry::IsValid(AssetHandle<T> handle) {
    if (auto manager = GetManager<T>(); manager != nullptr) {
        return manager->IsValid(handle);
    }
    return false;
}
template<typename T> requires IsPaladinAsset<T>
void AssetRegistry::Remove(AssetHandle<T> handle) {
    if (IsValid(handle)) {
        if (!dependencies[handle].empty()) {
            for (OpaqueAssetHandle& dependency : dependencies[handle]) {
                references[dependency].erase(handle);
                if (references[dependency].size()<1) {
                    RemoveOpaque(dependency);
                }
            }
        }
        auto manager = GetManager<T>();
        manager->RemoveAsset(handle);
    }
}

template<typename T> requires IsPaladinAsset<T>
IAssetManager<T> * AssetRegistry::GetManager() {
    if (auto it = asset_managers.find(typeid(T).hash_code()); it!=asset_managers.end()) {
        return static_cast<IAssetManager<T>*>(it->second.get());
    }
    return nullptr;
}
#endif //PALADIN_ASSETREGISTRY_H
