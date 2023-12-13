#pragma once

#include <types/cw_types.hpp>

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <boost/container/flat_map.hpp>

#include <span>

namespace cw {

namespace detail {

template<class Map, class StringLike>
Map
count_words_impl(std::span<StringLike const> seq) noexcept
{
    Map words_count;
    for (const auto& w : seq) [[likely]]
        ++words_count[w];
    return words_count;
}

} // namespace detail

CW_API WordsCountMap
count_words(std::span<std::string const> seq) noexcept;

CW_API WordsViewCountMap
count_words(std::span<std::string_view const> seq) noexcept;

inline auto
count_words_map(std::span<std::string const> seq)
{
    using Map = std::map<std::string, std::uint64_t>;
    return detail::count_words_impl<Map>(seq);
}

inline auto
count_words_flat_map(std::span<std::string const> seq)
{
    using Map = boost::container::flat_map<std::string, std::uint64_t>;
    return detail::count_words_impl<Map>(seq);
}

inline auto
count_words_absl_flat_hash_map(std::span<std::string const> seq)
{
    using Map = absl::flat_hash_map<std::string, std::uint64_t>;
    return detail::count_words_impl<Map>(seq);
}

inline auto
count_words_absl_node_hash_map(std::span<std::string const> seq)
{
    using Map = absl::node_hash_map<std::string, std::uint64_t>;
    return detail::count_words_impl<Map>(seq);
}

} // namespace cw
