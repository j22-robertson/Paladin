//
// Created by James Robertson on 10/02/2026.
//

#ifndef PALADIN_ASSETHANDLE_H
#define PALADIN_ASSETHANDLE_H
#include "utility/GenIndices.h"
struct OpaqueAssetHandle {
    GenKey inner;
    std::size_t type_id;
    ~OpaqueAssetHandle() = default;
    bool operator<(const OpaqueAssetHandle& rhs) const {
        return std::tie(inner.index,inner.generation, type_id) < std::tie(rhs.inner.index,rhs.inner.generation, rhs.type_id);
    }
    bool operator==(const OpaqueAssetHandle& rhs) const {
        return inner == rhs.inner &&this->type_id == rhs.type_id;
    }

};

template<typename T>
struct AssetHandle{
    GenKey inner;
    ~AssetHandle() = default;
    operator OpaqueAssetHandle() const {
        return OpaqueAssetHandle{inner, typeid(T).hash_code()};
    }
    bool operator==(const AssetHandle& rhs) const {
        return inner == rhs.inner;
    }


};

namespace std {
    template<typename T>
    struct hash<AssetHandle<T>> {
            size_t operator()(const AssetHandle<T>& h) const noexcept{
                std::uint64_t combined = (static_cast<std::uint64_t>(h.inner.index)<<32 | h.inner.generation);
                return std::hash<std::uint64_t>()(combined);
            }
    };
}

#endif //PALADIN_ASSETHANDLE_H