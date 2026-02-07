//
// Created by James Robertson on 07/02/2026.
//

#ifndef PALADIN_STRINGHASH_H
#define PALADIN_STRINGHASH_H
#include <random>
#include <string>
// Sources for implementation:
// https://lucassardois.medium.com/generational-indices-guide-8e3c5f7fd594
// http://www.isthe.com/chongo/tech/comp/fnv/index.html#public_domain


constexpr std::uint64_t FNV1AHash64(const std::string_view input_string) {
    constexpr std::uint64_t fnv1a_64_prime = 1099511628211;
    std::uint64_t offset_basis =  14695981039346656037;
    for (const char& c : input_string) {
        offset_basis ^= static_cast<std::uint64_t>(c);
        offset_basis *= fnv1a_64_prime;
    }
    return offset_basis; // Hashing result
}

constexpr std::uint32_t FNV1AHash32(const std::string_view& str) {
   constexpr std::uint32_t fnv1a_prime_32 = 16777619;
    std::uint32_t offset_basis = 2166136261;
    for (const char& c : str) {
        offset_basis ^= static_cast<std::uint32_t>(c);
        offset_basis *= fnv1a_prime_32;
    }
    return offset_basis;
}



/// Small test
/*
inline std::vector<std::string> GenerateTestStrings(std::size_t count, std::size_t min_length, std::size_t max_length) {
    static constexpr char characters[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "._-";
    static constexpr std::size_t characte_count = sizeof(characters)-1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> length_distribution(min_length,max_length);
    std::uniform_int_distribution<std::size_t> character_distribution(0,characte_count);

    std::vector<std::string> result;
    result.reserve(count);

    for (int i = 0; i < count; i++) {
        const auto len = length_distribution(gen);
        std::string generated;
        generated.reserve(len);

        for (int j = 0; j<len; j++) {
            generated.push_back(characters[character_distribution(gen)]);
        }
        result.push_back(std::move(generated));
    }
    return result;
}
auto test_strings = GenerateTestStrings(1000000,3,30);
std::vector<std::uint64_t> output_hashes;
std::unordered_map<std::uint64_t,std::string> output_hash_map;
PALADIN_LOG(INFO, "Start Hash")
for (std::string s : test_strings) {
auto hashed = FNV1AHash64(s);
auto [it,inserted] = output_hash_map.emplace(hashed, s);
if (!inserted && it->second != s) {
PALADIN_LOG(INFO, "Collision occured:"+std::to_string(hashed) + "String A:" + s + " Stringn B:" + it->second)
}
}
*/


#endif //PALADIN_STRINGHASH_H