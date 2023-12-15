#pragma once

#include <interfaces/words_counter.hpp>
#include <types/cw_types.hpp>


namespace cw {

using CountWordsFuncImpl = cw::WordsCountResult(std::span<std::filesystem::path const> files_range,
                                                cw::AlgorithmOptions const& opts);

template<std::add_pointer_t<CountWordsFuncImpl> FuncImpl>
class WordsCounterFromFreeFunction final : public cw::WordsCounter {
   private:
    cw::WordsCountResult do_count_words(std::span<std::filesystem::path const> files_range,
                                        cw::AlgorithmOptions const& opts) const override
    {
        return (*FuncImpl)(files_range, opts);
    }
};

} // namespace cw
