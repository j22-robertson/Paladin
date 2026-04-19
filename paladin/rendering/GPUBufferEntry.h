//
// Created by James Robertson on 19/04/2026.
//

#ifndef PALADIN_GPUBUFFERENTRY_H
#define PALADIN_GPUBUFFERENTRY_H
#include <cstdint>

/// An entry for a bindless buffer to be located within a shader
struct GPUBufferEntry {
    std::uint32_t heap_identifier;
    std::uint32_t aligned_size_bytes;
};


#endif //PALADIN_GPUBUFFERENTRY_H