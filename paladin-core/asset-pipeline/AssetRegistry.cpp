//
// Created by James Robertson on 07/02/2026.
//

#include "AssetRegistry.h"

template<typename T> requires IsPaladinAsset<T>
T* AssetRegistry::GetAsset(AssetID id) {
    //TODO: There must be a nicer way, using constexpr to generate name of type?
    return asset_managers.at(FNV1AHash32(typeid(T).name())).GetAsset(id);
}
