#include <kgui/widget.h>
#include <klib/mem_util.h>
namespace gui {


    widget::widget(){
    }
    void widget_list::init(uint64_t length){
        list = (gui::widget**)sys::service_malloc(length * sizeof (uint64_t));
        for(uint64_t i = 0; i < length; i++){
            list[i] = nullptr;
        }
        list_length = length;
    }

    void widget_list::add_widget(widget* widget){
        for(uint64_t i = 0; i < list_length; i++){
            if(list[i] == nullptr){
                list[i] = widget;
                return;
            }
        }
    }

    void widget_list::update_all(){

        for(uint64_t i = 0; i < list_length; i++){
            if(list[i] != nullptr){
                list[i]->update_widget();
            }
        }
    }
    void widget_list::draw_all(sys::graphic_context& context){

        for(uint64_t i = 0; i < list_length; i++){
            if(list[i] != nullptr){
                list[i]->draw_widget(context);
            }
        }
    }
}
