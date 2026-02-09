//
// Created by James Robertson on 08/02/2026.
//

#ifndef PALADIN_GENINDICES_H
#define PALADIN_GENINDICES_H
#include <assert.h>
#include <cstdint>
#include <vector>

constexpr std::uint32_t invalid = std::numeric_limits<std::uint32_t>::max();

// Index + Generation
struct GenKey {
    std::uint32_t index = invalid;
    std::uint32_t generation = invalid;
};

class GenerationalIndexAllocator {
    struct Generation {
        std::uint32_t current = invalid;
    };
    std::vector<Generation> m_generation_entries;
    std::vector<std::uint32_t> m_free_indices;

public:
    // Generate a new key, use previous generation entry as new generation if free indices != empty
    GenKey Allocate() {
        if (!m_free_indices.empty()) {
            const std::uint32_t index = m_free_indices.back();
            m_free_indices.pop_back();
            return {index,m_generation_entries[index].current};
        }
        m_generation_entries.push_back({0});
        return {static_cast<std::uint32_t>(m_generation_entries.size()-1),0};
    }

    // Increments generation and pushes back the key index
    void Deallocate(const GenKey key) {
        if (!IsValid(key)) return;
        ++m_generation_entries[key.index].current;
        m_free_indices.push_back(key.index);
    }

    // if key index is within generation entries and matches generation then true else false
    bool IsValid(const GenKey key) const {
        return key.index < m_generation_entries.size() && key.generation == m_generation_entries[key.index].current;
    }
};







#endif //PALADIN_GENINDICES_H