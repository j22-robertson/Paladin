//
// Created by jalr on 26-02-2026.
//

#ifndef PALADIN_RESOURCE_H
#define PALADIN_RESOURCE_H
#include <type_traits>

class GPUResource{};


template<typename T>
concept IsPaladinResource= std::is_base_of_v<GPUResource, T>;

#endif //PALADIN_RESOURCE_H