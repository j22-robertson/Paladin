//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_ASSETMANAGER_H
#define PALADIN_ASSETMANAGER_H
#include "utility/StringHash.h"
#include <unordered_map>
#include <unordered_set>
#include "asset/Asset.h"

template <typename T>
requires IsPaladinAsset<T>
class AssetManager {
public:
    AssetManager() = default;
    T* GetAsset(AssetID id) {
        if (assets.contains(id)) return assets.at(id).get();
        return false;
    }

private:
    bool lazy_load = false;

    std::unordered_map<AssetID, std::unique_ptr<Asset>> assets;

    std::unordered_set<AssetID> loaded_assets;
};



#endif //PALADIN_ASSETMANAGER_H
