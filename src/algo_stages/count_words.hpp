#pragma once

#include <types/cw_types.hpp>

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_node_map.hpp>
#include <boost/container/flat_map.hpp>

#include <map>
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
count_words_boost_unordered_flat_hash_map(std::span<std::string const> seq)
{
    using Map = boost::unordered_flat_map<std::string, std::uint64_t>;
    return detail::count_words_impl<Map>(seq);
}

inline auto
count_words_boost_unordered_node_map(std::span<std::string const> seq)
{
    using Map = boost::unordered_node_map<std::string, std::uint64_t>;
    return detail::count_words_impl<Map>(seq);
}

} // namespace cw
