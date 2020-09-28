#include <stdint.h>
namespace sys {

    struct graphic_raw_request{
        uint8_t request_raw_data[256]; // 256 o = 32 uint64_t i think we have a lot
    };
    struct create_graphic_window{
        const char* name;
        uint64_t width;
        uint64_t height;
        bool resizable = false; // will not be used
    };
    struct individual_request{
        uint64_t window_handler_code;
    };

    enum GRAPHIC_SYSTEM_REQUEST{
        NULL = 0,
        CREATE_WINDOW = 1,
        GET_WINDOW_BACK_BUFFER = 2,
        SWAP_WINDOW_BUFFER = 3,
        GET_WINDOW_POSITION = 4,
        GET_WINDOW_RESIZE = 5,
        GET_WINDOW_EVENT = 6
    };

    // window event are not used for the moment
    enum WINDOW_EVENT{
        EVENT_RESIZE = 0,
        EVENT_MOVE = 1,
    };

    struct graphic_system_service_protocol{
        uint16_t request_type;

        union request_info{
            graphic_raw_request raw_information; // used for later, size : 256o (2048 byte)
            create_graphic_window create_window_info;
            individual_request request;
        };
    };
}
