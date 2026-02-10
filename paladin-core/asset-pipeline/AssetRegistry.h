//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_ASSETREGISTRY_H
#define PALADIN_ASSETREGISTRY_H
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



class AssetRegistry {
public:
    AssetRegistry()
    {
        asset_importers.insert({typeid(Texture2DAsset).hash_code(),std::make_unique<TextureImporter>(*this)});
        asset_importers.insert({typeid(ModelAsset).hash_code(),std::make_unique<ModelImporter>(*this)});
    }


    template<typename T> requires IsPaladinAsset<T>
    T* GetAsset(AssetHandle<T> handle);

    template<typename T> requires IsImportable<T>
    AssetHandle<T> ImportAsset(std::string file);

private:

    template<typename T> requires IsImportable<T>
    IAssetImporter<T>* GetImporter();

    template<typename T> requires IsPaladinAsset<T>
    std::vector<AssetHandle<T>> InsertAssets(std::vector<std::unique_ptr<T>> assets);

    template<typename T> requires IsPaladinAsset<T>
    AssetHandle<T> InsertAsset(std::unique_ptr<T> asset);

    template<typename T> requires IsPaladinAsset<T>
    IAssetManager<T>* GetManager();

    std::unordered_map<std::size_t, AssetManager> asset_managers;
    std::unordered_map<std::size_t, AssetImporter> asset_importers{};
};


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
}

template<typename T> requires IsPaladinAsset<T>
AssetHandle<T> AssetRegistry::InsertAsset(std::unique_ptr<T> asset) {
    if (auto manager = GetManager<T>(); manager != nullptr) {
        return manager->InsertAsset(std::move(asset));
    }
    return {};
}

template<typename T> requires IsPaladinAsset<T>
IAssetManager<T> * AssetRegistry::GetManager() {
    if (auto it = asset_managers.find(typeid(T).hash_code()); it!=asset_managers.end()) {
        return static_cast<IAssetManager<T>*>(it->second.get());
    }
    return nullptr;
}
#endif //PALADIN_ASSETREGISTRY_H
