#include "stdint.h"
#include "ei_types.h"
#include "ei_utilsfct.h"
#include "hw_interface.h"

uint32_t ei_map_rgba (ei_surface_t surface,
                      ei_color_t   color)
{
    int ir, ig, ib, ia;
    hw_surface_get_channel_indices(surface, &ir, &ig, &ib, &ia);
    uint32_t col = 0;
    col += color.red << (8 * ir);
    col += color.green << (8 * ig);
    col += color.blue << (8 * ib);
    if (ia != -1) col += color.alpha << (8 * ia);
    return col;
}

void ei_fill (      ei_surface_t  surface,
                    const ei_color_t   *color,
                    const ei_rect_t    *clipper)
{
    uint32_t *addr = (uint32_t *)hw_surface_get_buffer(surface);
    int x0, y0, width, height, tot_width;
    ei_size_t size = hw_surface_get_size(surface);
    tot_width = size.width;
    if (clipper != NULL) {
        x0 = clipper -> top_left.x;
        y0 = clipper -> top_left.y;
        width = clipper -> size.width;
        height = clipper -> size.height;
    } else {
        x0 = 0;
        y0 = 0;
        width = size.width;
        height = size.height;
    }

    ei_color_t col = {0, 0, 0, 0xff};
    if (color != NULL) {
        col = *color;
    }

    int ir, ig, ib, ia;
    hw_surface_get_channel_indices(surface, &ir, &ig, &ib, &ia);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            uint32_t index = (y0 + i) * tot_width + x0 + j;
            ei_color_t pre_col = color_from_uint32(addr[index], ir, ig, ib, ia);
            ei_color_t mix_col = {0, 0, 0, 0};

            mix_col.red = (col.red * col.alpha + pre_col.red * (255 - col.alpha)) / 255;
            mix_col.green = (col.green * col.alpha + pre_col.green * (255 - col.alpha)) / 255;
            mix_col.blue = (col.blue * col.alpha + pre_col.blue * (255 - col.alpha)) / 255;
            mix_col.alpha = 0xff;

            addr[index] = ei_map_rgba(surface, col);
        }
    }
}



int ei_copy_surface (      ei_surface_t  destination,
                           const ei_rect_t    *dst_rect,
                           ei_surface_t  source,
                           const ei_rect_t    *src_rect,
                           ei_bool_t     alpha)
{

    if (dst_rect->size.width != src_rect->size.width || dst_rect->size.height != src_rect->size.height) return 1;

    uint32_t *dst_addr = (uint32_t *) hw_surface_get_buffer(destination);
    uint32_t *src_addr = (uint32_t *) hw_surface_get_buffer(source);

    ei_size_t dst_size = hw_surface_get_size(destination);
    ei_size_t src_size = hw_surface_get_size(source);

    int dst_r, dst_g, dst_b, dst_a;
    int src_r, src_g, src_b, src_a;

    hw_surface_get_channel_indices(destination, &dst_r, &dst_g, &dst_b, &dst_a);
    hw_surface_get_channel_indices(source, &src_r, &src_g, &src_b, &src_a);

    for (int i = 0; i < src_rect->size.height; i++) {
        for (int j = 0; j < src_rect->size.width; j++) {
            int src_index = (i + src_rect->top_left.y) * src_size.width + j + src_rect->top_left.x;
            int dst_index = (i + dst_rect->top_left.y) * dst_size.width + j + dst_rect->top_left.x;
            ei_color_t src_col = color_from_uint32(src_addr[src_index], src_r, src_g, src_b, src_a);
            ei_color_t dst_col = color_from_uint32(dst_addr[dst_index], dst_r, dst_g, dst_b, dst_a);
            ei_color_t mix_col = {0, 0, 0, 0};
            if (alpha == EI_TRUE) {
                mix_col.red = (src_col.red * src_col.alpha + dst_col.red * (255 - src_col.alpha)) / 255;
                mix_col.green = (src_col.green * src_col.alpha + dst_col.green * (255 - src_col.alpha)) / 255;
                mix_col.blue = (src_col.blue * src_col.alpha + dst_col.blue * (255 - src_col.alpha)) / 255;
                mix_col.alpha = 0xff;
            } else {
                mix_col = src_col;
            }
            dst_addr[dst_index] = ei_map_rgba(destination, mix_col);
        }
    }

    return 0;
}
