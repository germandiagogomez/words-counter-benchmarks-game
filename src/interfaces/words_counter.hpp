#pragma once

#include <count_words/cw_api.hpp>
#include <types/cw_types.hpp>

#include <filesystem>
#include <span>
#include <vector>

namespace cw {

class CW_API WordsCounter {
   public:
    WordsCountResult count_words(std::span<std::filesystem::path> filenames,
                                 AlgorithmOptions const& opts) const
    {
        return do_count_words(filenames, opts);
    }
    virtual ~WordsCounter() = default;

   private:
    virtual WordsCountResult do_count_words(std::span<std::filesystem::path const> filenames,
                                            AlgorithmOptions const& opts) const = 0;
};

using WordsCounterPtr = std::shared_ptr<cw::WordsCounter>;

} // namespace cw
