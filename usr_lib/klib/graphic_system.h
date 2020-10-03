#include <stdint.h>
namespace sys {
    // FOR GRAPHIC MESSAGE :
    struct graphic_raw_request{
        uint8_t request_raw_data[256]; // 256 o = 32 uint64_t i think we have a lot
    }__attribute__((packed));
    struct create_graphic_window{
        char* name;
        uint64_t width;
        uint64_t height;
        bool resizable = false; // will not be used
    }__attribute__((packed));
    struct individual_request{
        uint64_t window_handler_code;
    }__attribute__((packed));

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

        union{
            graphic_raw_request raw_information; // used for later, size : 256o (2048 byte)
            create_graphic_window create_window_info;
            individual_request get_request;
        };
    }__attribute__((packed));
    // FOR OTHER THING
    struct pixel{
        union{
            struct {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
            uint32_t pix;
        };
        pixel(uint32_t p){
            pix = p;
        };
        pixel(uint8_t vr, uint8_t vg, uint8_t vb, uint8_t va =  255){
            r = vr;
            g = vg;
            b = vb;
            a = va;
        }
    };



    class graphic_context{
        uint64_t context_width;
        uint64_t context_height;
        uint64_t wid = 0;
        pixel* back_buffer;
        char* context_name;
    public:
        graphic_context(uint64_t width, uint64_t height, const char* name);

        void clear_buffer(const pixel color);
        void swap_buffer();

    };
}
