#include <algo_stages/read_input.hpp>
#include <boost/container/small_vector.hpp>
// #define BOOST_ASIO_ENABLE_BUFFER_DEBUGGING
// #define BOOST_ASIO_ENABLE_HANDLER_TRACKING
#if USE_ASYNC_FILE_IO
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>
#endif

namespace cw {

// IO for reading directory entries
std::vector<std::filesystem::path>
read_filenames(const std::filesystem::path& direc)
{
    std::vector<std::filesystem::path> filenames;
    for (const auto& p : std::filesystem::directory_iterator{direc}) {
        if (p.path().extension() == ".txt")
            filenames.push_back(p);
    }
    return filenames;
}

FastDynamicBuffer
read_file_zero_copy(const std::filesystem::path& p)
{
    return FastDynamicBuffer(boost::iostreams::mapped_file(p.c_str()));
}

DynamicBuffer
read_file(const std::filesystem::path& p)
{
    boost::iostreams::mapped_file file(p.c_str());
    DynamicBuffer buffer(file.size());
    auto b = reinterpret_cast<std::byte*>(std::addressof(*file.begin()));
    auto e = reinterpret_cast<std::byte*>(std::addressof(*file.end()));
    std::copy(b, e, std::begin(buffer));
    return buffer;
}

DynamicBuffer
read_files(std::span<std::filesystem::path const> paths)
{
    using Vec = boost::container::small_vector<boost::iostreams::mapped_file, 16>;
    Vec files;
    std::size_t total_bytes = 0;
    for (const auto& p : paths) {
        files.push_back(boost::iostreams::mapped_file(p.c_str()));
        total_bytes += files.back().size();
    }

    DynamicBuffer buffer(total_bytes + files.size());
    for (std::size_t offset = 0; const auto& f : files) {
        auto b = reinterpret_cast<std::byte*>(std::addressof(*f.begin()));
        auto e = reinterpret_cast<std::byte*>(std::addressof(*f.end()));
        std::copy(b, e, std::begin(buffer) + offset);
        offset += f.size();
        buffer[offset] = std::byte{' '};
        ++offset;
    }
    return buffer;
}

#if USE_ASYNC_FILE_IO
boost::asio::awaitable<boost::container::vector<std::byte>>
read_file_async(boost::asio::any_io_executor& exe, const std::filesystem::path& p)
{
    using boost::asio::use_awaitable;
    boost::asio::stream_file file(exe,
                                  p.c_str(),
                                  boost::asio::file_base::read_only |
                                      boost::asio::file_base::exclusive);

    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Could not open {}", p.c_str()));
    }
    constexpr std::size_t block_size = 1024 * 1024 * 2;
    boost::container::vector<std::byte> buf(block_size, boost::container::default_init);
    std::size_t curr_pos = 0;
    std::size_t bytes_read = 1;
    std::vector<std::byte> v(block_size);
    while (bytes_read > 0) {
        try {
            bytes_read = co_await file.async_read_some(
                boost::asio::buffer(buf.data() + curr_pos, buf.size() - curr_pos),
                use_awaitable);
            curr_pos += bytes_read;
            if (curr_pos == buf.size())
                buf.resize(buf.size() * 2, boost::container::default_init);
        } catch (const boost::system::system_error& e) {
            fmt::print("{}\n", e.what());
            throw;
        }
    }
    buf.resize(curr_pos, boost::container::default_init);
    buf.shrink_to_fit();
    co_return std::move(buf);
}
#endif


}