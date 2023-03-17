#ifndef PROJETC_IG_EI_STRUCT_FOR_BIND_H
#define PROJETC_IG_EI_STRUCT_FOR_BIND_H

#include "ei_event.h"
#include "ei_widget.h"

typedef struct ei_hooked_callback_t {

    ei_eventtype_t eventtype;
    ei_widget_t *widget;
    ei_tag_t tag;
    ei_callback_t callback;
    void *user_param;
    struct ei_hooked_callback_t *next;

} ei_hooked_callback_t;


ei_hooked_callback_t** seek_the_struct(void);


void alloc_the_struct(void);
#endif //PROJETC_IG_EI_STRUCT_FOR_BIND_H
