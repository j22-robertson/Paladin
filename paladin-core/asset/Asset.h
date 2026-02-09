//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_ASSET_H
#define PALADIN_ASSET_H
#include <cstdint>
#include <typeinfo>
#include "utility/StringHash.h"
#include <concepts>
#include "utility/GenIndices.h"

using AssetHandle = GenKey;
class Asset {
public:

private:
};

template<typename T>
concept IsPaladinAsset = std::is_base_of_v<Asset, T>;

#endif //PALADIN_ASSET_H