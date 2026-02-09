//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_IASSETIMPORTER_H
#define PALADIN_IASSETIMPORTER_H

#include "IAssetImporterBase.h"

class AssetRegistry;

template<typename T>
requires IsPaladinAsset<T>
class IAssetImporter : public IAssetImporterBase
{
public:
    IAssetImporter() = delete;
    ~IAssetImporter() override = default;


    IAssetImporter(AssetRegistry& asset_registry);
    IAssetImporter(IAssetImporter const&) = default;
    AssetRegistry* m_asset_registry = nullptr;

};

template <typename T> requires IsPaladinAsset<T>
IAssetImporter<T>::IAssetImporter(AssetRegistry& asset_registry)
{
    m_asset_registry = &asset_registry;
}


#endif //PALADIN_IASSETIMPORTER_H
