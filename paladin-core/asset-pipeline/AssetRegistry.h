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
using AssetManager = std::unique_ptr<IAssetManager<Asset>>;


using AssetImporter = std::unique_ptr<IAssetImporterBase>;



class AssetRegistry {
public:
    AssetRegistry()
    {
        asset_importers.insert({typeid(Texture2DAsset).hash_code(),std::make_unique<TextureImporter>(*this)});
        asset_importers.insert({typeid(ModelAsset).hash_code(),std::make_unique<ModelImporter>(*this)});
    }


    template<typename T> requires IsPaladinAsset<T>
    T* GetAsset(AssetHandle handle);

    template<typename T> requires IsPaladinAsset<T>
    AssetHandle LoadAsset(std::string file) const;

private:
    std::unordered_map<std::size_t, AssetManager> asset_managers;
    std::unordered_map<std::size_t, AssetImporter> asset_importers{};
};


template<typename T> requires IsPaladinAsset<T>
T* AssetRegistry::GetAsset(AssetHandle handle) {
    //TODO: There must be a nicer way, using constexpr to generate name of type?
    return asset_managers.at(typeid(T).hash_code())->GetAsset(handle);
}
template<typename T> requires IsPaladinAsset<T>
AssetHandle AssetRegistry::LoadAsset(std::string file) const
{
    std::string type_name = typeid(T).name();
    PALADIN_LOG(INFO, "Finding Importer for Type:"+type_name)
    if (asset_importers.contains(typeid(T).hash_code()))
    {
        PALADIN_LOG(INFO, "Found Importer for Type:"+type_name)
        return asset_importers.at(typeid(T).hash_code())->LoadAsset(file);
    }
    return{};
}
#endif //PALADIN_ASSETREGISTRY_H
