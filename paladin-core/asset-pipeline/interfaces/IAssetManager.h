//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_IASSETMANAGER_H
#define PALADIN_IASSETMANAGER_H
#include "asset/Asset.h"
#include <unordered_map>
#include <unordered_set>

template<typename T>
requires IsPaladinAsset<T>
class IAssetManager
{
public:
    T* GetAsset(AssetHandle handle)
    {
        return assets[handle.index].get();
    }



private:
    bool lazy_load = false;

    std::unordered_map<std::uint32_t, std::unique_ptr<T>> assets;

    std::unordered_set<std::uint32_t> loaded_assets;
};

#endif //PALADIN_IASSETMANAGER_H