//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_ASSETREGISTRY_H
#define PALADIN_ASSETREGISTRY_H

#include "AssetManager.h"

class AssetRegistry {


public:
    template<typename T> requires IsPaladinAsset<T>
    T* GetAsset(AssetID id);

private:
    std::unordered_map<std::uint32_t, AssetManager<Asset>> asset_managers = std::unordered_map<std::uint32_t, AssetManager<Asset>>();
};



#endif //PALADIN_ASSETREGISTRY_H
