//
// Created by reniaudf on 05/05/2022.
//

#ifndef PROJETC_IG_EI_BUTTON_H
#define PROJETC_IG_EI_BUTTON_H

#include "ei_frame.h"

typedef struct ei_button {
    ei_frame frame;
    int corner_radius;
    ei_callback_t callback;
    void * user_param;
} ei_button;

#endif //PROJETC_IG_EI_BUTTON_H
