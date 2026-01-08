#pragma once

#include "libcore/str_writer.hpp"
#include <stdint.h>
#include <string.h>
#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/space.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/str.hpp"
#include "math/align.hpp"
#include "protocols/init/init.hpp"
namespace prot
{

struct FileInfo
{
    uint64_t size;

    uint64_t created_at;
    uint64_t modified_at;
    uint64_t accessed_at;
    uint16_t is_directory;
    core::WStr name;
};

struct DirListEntry
{
    core::WStr name;
    bool is_directory;
};

struct DirList
{
    core::Vec<DirListEntry> entries;
};
enum FsFileMessageType
{

    FS_GET_INFO = 0,
    // file operations
    FS_READ = 1,
    FS_WRITE = 2,
    FS_CLOSE = 3,
    // directory operations

    FS_LIST_DIR = 13,
    FS_OPEN_FILE = 15, // open a file from the current file (directory)

    // to implement
    FS_RENAME = 10,
    FS_SET_INFO = 12,

    FS_CREATE_DIR = 14,

};

struct FsFileCacheEntry
{
    uint64_t offset;
    uint64_t size;
    Wingos::MemoryAsset asset;
    Wingos::VirtualMemoryAsset mapped;
    int score;
};
class FsFile
{
    Wingos::IpcClient connection;
    bool keep_alive = false;
    core::Vec<FsFileCacheEntry> cache_entries;


    void add_cache_entry(uint64_t offset, uint64_t size, Wingos::MemoryAsset &asset, Wingos::VirtualMemoryAsset &mapped)
    {
        FsFileCacheEntry entry = {};
        entry.offset = offset;
        entry.size = size;
        entry.asset = asset;
        entry.mapped = mapped;
        entry.score = 1;

        if(cache_entries.len() > 256)
        {
            // evict lowest score
            size_t lowest_score_index = 0;
            int lowest_score = cache_entries[0].score;

            for (size_t i = 1; i < cache_entries.len(); i++)
            {
                if (cache_entries[i].score < lowest_score)
                {
                    lowest_score = cache_entries[i].score;
                    lowest_score_index = i;
                }
            }

            Wingos::Space::self().release_asset( cache_entries[lowest_score_index].mapped);
            Wingos::Space::self().release_asset( cache_entries[lowest_score_index].asset);
            cache_entries[lowest_score_index] = entry;
        }
        else
        {

            cache_entries.push(entry);
        }
    }

public:

    ~FsFile () {
        for(size_t i = 0; i < cache_entries.len(); i++)
        {
            Wingos::Space::self().release_asset( cache_entries[i].mapped);
            Wingos::Space::self().release_asset( cache_entries[i].asset);
        }
        cache_entries.clear();
    }

    // Disable copy to prevent double-close issues
    FsFile(const FsFile&) = delete;
    FsFile& operator=(const FsFile&) = delete;

    // Enable move
    FsFile(FsFile&& other)
        : connection(other.connection)
        , keep_alive(other.keep_alive)
        , cache_entries(core::move(other.cache_entries))
    {
    }

    FsFile& operator=(FsFile&& other)
    {
        if (this != &other)
        {
            // Clean up our existing cache entries
            for(size_t i = 0; i < cache_entries.len(); i++)
            {
                Wingos::Space::self().release_asset( cache_entries[i].mapped);
                Wingos::Space::self().release_asset( cache_entries[i].asset);
            }
            cache_entries.clear();

            core::swap(connection, other.connection);
            core::swap(keep_alive, other.keep_alive);

            cache_entries = core::move(other.cache_entries);
        }
        return *this;
    }

    FsFile() = default;

    Wingos::IpcClient &raw_client() { return connection; }

    static core::Result<FsFile> connect(IpcServerHandle fs_endpoint, bool keep_alive = false)
    {
        log::log$("FsFile::connect: connecting to server endpoint {}", fs_endpoint);
        FsFile file = {};
        file.connection = Wingos::Space::self().connect_to_ipc_server(fs_endpoint);
        log::log$("FsFile::connect: created connection handle {}, waiting for accept...", file.connection.handle);
        file.keep_alive = keep_alive;
        file.connection.wait_for_accept();
        log::log$("FsFile::connect: connection {} accepted", file.connection.handle);

        return (file);
    }

    core::Result<size_t> read(Wingos::MemoryAsset &asset, size_t offset, size_t len)
    {
        IpcMessage message = {};
        message.data[0].data = FS_READ;
        message.data[1].data = offset;
        message.data[2].data = len;
        message.data[3].is_asset = true;
        message.data[3].asset_handle = asset.handle;
        message.data[4].data = 0;
        auto msg = try$(connection.call(message));
        size_t received_len = msg.data[1].data;
        asset = Wingos::MemoryAsset::from_handle(msg.data[2].asset_handle);
        return received_len;
    }



    core::Result<size_t> read(void* buffer, size_t offset, size_t len)
    {
        if (len == 0)
        {
            return core::Result<size_t>::success(0);
        }

        for(size_t i = 0; i < cache_entries.len(); i++)
        {
            auto &entry = cache_entries[i];
            if(offset >= entry.offset && (offset + len) <= (entry.offset + entry.size))
            {
                size_t cache_offset = offset - entry.offset;
                memcpy(buffer, (void *)((uintptr_t)entry.mapped.ptr() + cache_offset), len);
                entry.score++;
                return len;
            }
        }

        size_t aoffset = math::alignDown(offset, 4096ul);

        size_t alen = math::alignUp(len*2 + offset, 4096ul) - aoffset;

        size_t delta_offset = offset - aoffset;
        Wingos::MemoryAsset masset = Wingos::Space::self().allocate_physical_memory(alen);


        auto res = try$(this->read(masset, aoffset, alen));

        Wingos::VirtualMemoryAsset mapped = Wingos::Space::self().map_memory(masset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);
        memcpy(buffer, (void *)((uintptr_t)mapped.ptr() + delta_offset), len);

        this->add_cache_entry(aoffset, alen, masset, mapped);
       // Wingos::Space::self().release_asset(mapped);
      //  Wingos::Space::self().release_asset(masset);
        return res;
    }

    core::Result<size_t> write(Wingos::MemoryAsset &asset, size_t offset, size_t len)
    {
        for(size_t i = 0; i < cache_entries.len(); i++)
        {
            auto &entry = cache_entries[i];
            if(offset >= entry.offset && (offset + len) <= (entry.offset + entry.size))
            {
                size_t cache_offset = offset - entry.offset;
                memcpy((void *)((uintptr_t)entry.mapped.ptr() + cache_offset), (void *)((uintptr_t)asset.memory.start() + cache_offset), len);
                entry.score++;
            }
        }
        IpcMessage message = {};
        message.data[0].data = FS_WRITE;
        message.data[1].data = offset;
        message.data[2].data = len;
        message.data[3].is_asset = true;
        message.data[3].asset_handle = asset.handle;
        message.data[4].data = 0;
        auto msg = try$(connection.call(message));
        size_t received_len = msg.data[1].data;
        return received_len;
    }


    core::Result<size_t> write(void* buffer, size_t offset, size_t len)
    {
        if (len == 0)
        {
            return core::Result<size_t>::success(0);
        }

        Wingos::MemoryAsset masset = Wingos::Space::self().allocate_physical_memory(len);

        Wingos::VirtualMemoryAsset mapped = Wingos::Space::self().map_memory(masset, ASSET_MAPPING_FLAG_READ | ASSET_MAPPING_FLAG_WRITE);

        memcpy(mapped.ptr(), buffer, len);
        auto res = try$(this->write(masset, offset, len));
        Wingos::Space::self().release_asset(mapped);
        Wingos::Space::self().release_asset(masset);
        return res;
    }

    core::Result<void> close()
    {
        // Just disconnect - the IPC disconnect notification will tell the server to clean up.
        // Don't send FS_CLOSE AND disconnect as this creates a race condition where the server
        // might receive both FS_CLOSE and IPC_MESSAGE_FLAG_DISCONNECT and try to clean up twice.
        connection.disconnect();
        return {};
    }

    core::Result<FileInfo> get_info()
    {
        IpcMessage message = {};
        message.data[0].data = FS_GET_INFO;
        auto msg = try$(connection.call(message));

        FileInfo info = {};
        info.size = msg.data[1].data;
        info.created_at = msg.data[2].data;
        info.modified_at = msg.data[3].data;
        info.accessed_at = msg.data[4].data;
        info.is_directory = msg.data[5].data; 
        size_t name_len = msg.len;
        char name_buf[110] = {0};
        for (size_t i = 0; i < name_len && i < 110; i++)
        {
            name_buf[i] = msg.raw_buffer[i];
        }
        info.name = core::WStr::copy(core::Str(name_buf, name_len));

        return info;
    }

    core::Result<DirListEntry> list_dir_entry(size_t index)
    {
        IpcMessage message = {};
        message.data[0].data = FS_LIST_DIR;
        message.data[1].data = index;
        auto sended_message = connection.send(message, true);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send list dir entry message");
        }

        while (true)
        {
            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = core::move(received.unwrap());

                DirListEntry entry;
                size_t name_len = msg.len;
                char name_buf[110] = {0};
                for (size_t i = 0; i < name_len && i < 110; i++)
                {
                    name_buf[i] = msg.raw_buffer[i];
                }
                entry.name = core::WStr::copy(core::Str(name_buf, name_len));
                entry.is_directory = msg.data[1].data != 0;

                return entry;
            }
        }
    }
    core::Result<DirList> list_dir()
    {

        auto l = this->get_info();
        if (l.is_error())
        {
            return l.error();
        }

        FileInfo info = core::move(l.unwrap());
        if (!info.is_directory)
        {
            return ("not a directory");
        }

        DirList dir_list;

        for (size_t i = 0; i < info.size; i++)
        {
            auto entry_res = this->list_dir_entry(i);
            if (entry_res.is_error())
            {
                break;
            }
            dir_list.entries.push(core::move(entry_res.unwrap()));
        }

        return dir_list;
    }

    core::Result<FsFile> open_file(core::Str path)
    {
        IpcMessage message = {};
        message.data[0].data = FS_OPEN_FILE;
        size_t path_len = path.len();
        if (path_len > 100)
        {
            return ("path too long");
        }
        for (size_t i = 0; i < path_len; i++)
        {
            message.raw_buffer[i] = path[i];
        }
        message.len = path_len;

        auto received = connection.call(message);

        auto msg = core::move(received.unwrap());


        if(msg.data[0].data == 0)
        {
            return ("failed to open file");
        }

        IpcServerHandle file_endpoint = msg.data[1].data;
        auto file_res = FsFile::connect(file_endpoint);
        return file_res;
    }
};

}; // namespace prot
