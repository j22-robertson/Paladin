//
// Created by jalr on 26-02-2026.
//

#ifndef PALADIN_BASERESOURCEMANAGER_H
#define PALADIN_BASERESOURCEMANAGER_H
#include "utility/GenIndices.h"
#include "GPUResource.h"
#include "Logger.h"
#include "asset/AssetHandle.h"
#include "profiling/Profiling.h"


class ResourceManager
{
public:
    ResourceManager() = default;
};

template<typename T>
    requires IsPaladinResource<T>
class GPUResourceHandle
{
public:
    GenKey inner;
};


template<typename T>
    requires IsPaladinResource<T>
class IResourceManager : public ResourceManager {
    struct Generation {
        std::uint32_t current = invalid;
        std::unique_ptr<T> value = nullptr;
    };
    std::vector<Generation> m_generation_entries;
    std::vector<std::uint32_t> m_free_indices;

public:
    IResourceManager() = default;

    T* Get(OpaqueAssetHandle handle)
    {
        if (IsValid(handle.inner))
        {
            return m_generation_entries[handle.inner.index].value.get();
        }
        return nullptr;
    }

    GPUResourceHandle<T> Insert(std::unique_ptr<T> resource,OpaqueAssetHandle origin_handle) {
        auto [index, generation] = origin_handle.inner;

        if (origin_handle.inner.index>=m_generation_entries.size())
        {
            m_generation_entries.resize(index+1);
        }
        m_generation_entries[index].current = generation;
        m_generation_entries[index].value.reset(std::move(resource));
        return {origin_handle.inner};
    }

    // Increments generation and pushes back the key index
    void Deallocate(const GenKey key) {
        if (!IsValid(key)) return;
        auto& entry = m_generation_entries[key.index];
        if (entry.value != nullptr)
        {
            entry.value.reset();
        }
        ++entry.current;
        m_free_indices.push_back(key.index);
    }

    // if key index is within generationn entries and matches generation then true else false
    [[nodiscard]] bool IsValid(const GenKey key) const {
        return key.index < m_generation_entries.size() && key.generation == m_generation_entries[key.index].current;
    }
};



#endif //PALADIN_BASERESOURCEMANAGER_H