//
// Created by James Robertson on 10/02/2026.
//

#ifndef PALADIN_IASSETMANAGERBASE_H
#define PALADIN_IASSETMANAGERBASE_H

class IAssetManagerBase {
public:
    virtual ~IAssetManagerBase() = default;
    virtual void RemoveOpaque(OpaqueAssetHandle handle) = 0;
};



#endif //PALADIN_IASSETMANAGERBASE_H