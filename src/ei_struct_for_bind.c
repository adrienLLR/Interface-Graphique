#include <stdio.h>
#include <stdlib.h>

#include "ei_geometrymanager.h"
#include "ei_struct_for_bind.h"

ei_hooked_callback_t *struct_hook = NULL;

ei_hooked_callback_t** seek_the_struct(void)
{
    return &struct_hook;
}

void alloc_the_struct(void)
{
    struct_hook = malloc(sizeof(struct ei_hooked_callback_t));
}