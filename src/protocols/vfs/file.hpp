#pragma once

#include "iol/wingos/ipc.hpp"
#include "libcore/str.hpp"
#include "protocols/init/init.hpp"
#include "libcore/str_writer.hpp"
namespace prot
{


    struct FileInfo 
    {
        uint64_t size;

        uint64_t created_at;
        uint64_t modified_at;
        uint64_t accessed_at;
        bool is_directory;
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

class FsFile
{
    Wingos::IpcClient connection;


public:
    Wingos::IpcClient &raw_client() { return connection; }

    static core::Result<FsFile> connect(IpcServerHandle fs_endpoint)
    {
        FsFile file = {};
        file.connection = Wingos::Space::self().connect_to_ipc_server(fs_endpoint);
        file.connection.wait_for_accept();
        return core::Result<FsFile>::success(core::move(file));
    }


    
    core::Result<size_t> read(Wingos::MemoryAsset &asset, size_t offset, size_t len)
    {
        IpcMessage message = {};
        message.data[0].data = FS_READ;
        message.data[1].data = offset;
        message.data[2].data = len;
        message.data[3].is_asset = true;
        message.data[3].asset_handle = asset.handle;
        auto sended_message = connection.send(message, true);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send read file message");
        }

        while(true)
        {
            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = core::move(received.unwrap());
                size_t received_len = msg.data[1].data;
                asset = Wingos::MemoryAsset::from_handle(msg.data[2].asset_handle);
                return received_len;
            }
        }
    }

    core::Result<size_t> write(Wingos::MemoryAsset &asset, size_t offset, size_t len)
    {
        IpcMessage message = {};
        message.data[0].data = FS_WRITE;
        message.data[1].data = offset;
        message.data[2].data = len;
        message.data[3].is_asset = true;
        message.data[3].asset_handle = asset.handle;
        auto sended_message = connection.send(message, true);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send write file message");
        }

        while(true)
        {
            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = core::move(received.unwrap());
                size_t received_len = msg.data[1].data;
                return received_len;
            }
        }
    }

    core::Result<void> close()
    {
        IpcMessage message = {};
        message.data[0].data = FS_CLOSE;
        auto sended_message = connection.send(message, false);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send close file message");
        }

        (void)message_handle;
        return {};
    }


    core::Result<FileInfo> get_info()
    {
        IpcMessage message = {};
        message.data[0].data = FS_GET_INFO;
        auto sended_message = connection.send(message, true);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send get info message");
        }

        while(true)
        {
            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = core::move(received.unwrap());


                FileInfo info;
                info.size = msg.data[1].data;
                info.created_at = msg.data[2].data;
                info.modified_at = msg.data[3].data;
                info.accessed_at = msg.data[4].data;
                info.is_directory = msg.data[5].data != 0;
                size_t name_len = msg.len;
                char name_buf[110] = {0};
                for (size_t i = 0; i < name_len && i < 110; i++)
                {
                    name_buf[i] = msg.raw_buffer[i];
                }
                info.name = core::WStr::copy(core::Str(name_buf, name_len));

                return info;

            }
        }

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

        while(true)
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
        if(l.is_error())
        {
            return l.error();
        }

        FileInfo info = core::move(l.unwrap());
        if(!info.is_directory)
        {
            return ("not a directory");
        }

        DirList dir_list;

        for(size_t  i= 0; i < info.size; i++)
        {
            auto entry_res = this->list_dir_entry(i);
            if(entry_res.is_error())
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
        if(path_len > 100)
        {
            return ("path too long");
        }
        for(size_t i = 0; i < path_len; i++)
        {
            message.raw_buffer[i] = path[i];
        }
        message.len = path_len;

        auto sended_message = connection.send(message, true);
        auto message_handle = sended_message.unwrap();
        if (sended_message.is_error())
        {
            return ("failed to send open file message");
        }

        while(true)
        {
            auto received = connection.receive_reply(message_handle);
            if (!received.is_error())
            {
                auto msg = core::move(received.unwrap());

                if(msg.data[0].data == 0)
                {
                    return ("failed to open file");
                }

                IpcServerHandle file_endpoint = msg.data[1].data;
                auto file_res = FsFile::connect(file_endpoint);
                if(file_res.is_error())
                {
                    return file_res.error();
                }
                return file_res.unwrap();

            }
        }
    }


    

};

}; // namespace prot
