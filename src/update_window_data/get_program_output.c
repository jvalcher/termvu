#include <string.h>

#include "get_program_output.h"
#include "../data.h"
#include "../plugins.h"

static void get_program_output_gdb (state_t *state);



void
get_program_output (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_program_output_gdb (state);
            break;
    }
}



// TODO: no program output when stepping, check parser
static void
get_program_output_gdb (state_t *state)
{
    window_t *win;
    char     *src_ptr,
             *dest_ptr;

    win      = state->plugins[Prg]->win;
    src_ptr  = state->debugger->program_buffer;
    dest_ptr = win->buff_data->buff;

    // check buffer size
    if (strlen (src_ptr) < (win->buff_data->buff_len - strlen (dest_ptr) - 1)) {
        dest_ptr += strlen (dest_ptr);
    }

    // create buffer
    if (strstr (src_ptr, "error") == NULL) {

        while (*src_ptr != '\0') {
            *dest_ptr++ = *src_ptr++; 
        }
        *dest_ptr = '\0';

        win->buff_data->changed = true;
    }
}




