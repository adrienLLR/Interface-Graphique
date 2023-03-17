
#ifndef PROJETC_IG_EI_PLACER_H
#define PROJETC_IG_EI_PLACER_H

typedef struct ei_placer {
    ei_geometrymanager_t *manager;
    ei_anchor_t anchor;
    int         x;         int         y;
    int         width;     int         height;
    float       rel_x;     float       rel_y;
    float       rel_width; float       rel_height;
} ei_placer;

#endif //PROJETC_IG_EI_PLACER_H
