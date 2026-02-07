//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_ASSETMANAGER_H
#define PALADIN_ASSETMANAGER_H
#include <unordered_map>
#include <unordered_set>
using AssetID = std::uint64_t;

class AssetManager {

public:
private:
    std::unordered_set<AssetID> loaded_assets;
};



#endif //PALADIN_ASSETMANAGER_H
