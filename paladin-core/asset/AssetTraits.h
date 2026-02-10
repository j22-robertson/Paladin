//
// Created by James Robertson on 10/02/2026.
//
#ifndef PALADIN_ASSETTRAITS_H
#define PALADIN_ASSETTRAITS_H
#include "asset/Asset.h"

template<typename T> requires IsPaladinAsset<T>
struct HasImporter {
    static constexpr bool value = false;
};

template <typename T>
concept IsImportable = HasImporter<T>::value;


#endif //PALADIN_ASSETTRAITS_H