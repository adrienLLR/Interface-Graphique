#include <ei_event.h>
#include "ei_types.h"
#include "ei_widget.h"
#include "ei_struct_for_bind.h"

void ei_bind(ei_eventtype_t eventtype,
             ei_widget_t   *widget,
             ei_tag_t  	    tag,
             ei_callback_t  callback,
             void          *user_param) {
    {
        ei_hooked_callback_t **struct_hooked = seek_the_struct();
        if (*struct_hooked == NULL) {
            alloc_the_struct();
            (*struct_hooked) -> eventtype = eventtype;
            (*struct_hooked) -> widget = widget;
            (*struct_hooked) -> tag = tag;
            (*struct_hooked) -> callback = callback;
            (*struct_hooked) -> user_param = user_param;
            (*struct_hooked) -> next = NULL;
        } else {
            ei_hooked_callback_t *cellule_courante = *struct_hooked;
            while (cellule_courante -> next != NULL) {
                cellule_courante = cellule_courante -> next;
            }
            ei_hooked_callback_t *cellule_suivante = malloc(sizeof(struct ei_hooked_callback_t));
            cellule_suivante -> eventtype = eventtype;
            cellule_suivante -> widget = widget;
            cellule_suivante -> tag = tag;
            cellule_suivante -> callback = callback;
            cellule_suivante -> user_param = user_param;
            cellule_suivante -> next = NULL;

            cellule_courante -> next = cellule_suivante;
        }
    }
}

void ei_unbind(ei_eventtype_t eventtype,
               ei_widget_t   *widget,
               ei_tag_t  	    tag,
               ei_callback_t  callback,
               void          *user_param)
{
    ei_hooked_callback_t **struct_hooked = seek_the_struct();

    if ( *struct_hooked == NULL ) return;

    ei_hooked_callback_t *cellule_courante = NULL; /* C'est la sentinelle */
    ei_hooked_callback_t *cellule_suivante = *struct_hooked; /* La cellule suivante est la cellule que l'on va contrôler */

    while   ( cellule_suivante != NULL && (
              cellule_suivante -> eventtype != eventtype ||
              cellule_suivante -> widget != widget ||
              cellule_suivante -> callback != callback ||
              cellule_suivante -> user_param != user_param ))
    {
        cellule_courante = cellule_suivante;
        cellule_suivante = cellule_suivante -> next;
    }

    if ( cellule_suivante == NULL ) return;

    if ( cellule_courante == NULL) /* Correspond au cas où la première cellule va être unbind */
    {
        *struct_hooked = cellule_suivante -> next;
        free(cellule_suivante);
    }
    else
    {
        cellule_courante -> next = cellule_suivante -> next;
        free(cellule_suivante);
    }
}
