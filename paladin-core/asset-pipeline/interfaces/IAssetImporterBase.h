//
// Created by jalr on 09-02-2026.
//

#ifndef PALADIN_IASSETIMPORTERBASE_H
#define PALADIN_IASSETIMPORTERBASE_H
class IAssetImporterBase
{
public:
    virtual ~IAssetImporterBase() = default;
    virtual AssetHandle LoadAsset(std::string file) = 0;
};
#endif //PALADIN_IASSETIMPORTERBASE_H