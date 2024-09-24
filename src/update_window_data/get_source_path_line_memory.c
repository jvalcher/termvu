#include <ncurses.h>

#include "get_source_path_line_memory.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"
#include "../parse_debugger_output.h"

static int get_source_path_line_memory_gdb (state_t *state);
static int get_source_path_line_memory_pdb (state_t *state);



int
get_source_path_line_memory (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_source_path_line_memory_gdb (state) == RET_FAIL) {
                pfemr ("Failed to get source, line, memory (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            if (get_source_path_line_memory_pdb (state) == RET_FAIL) {
                pfemr ("Failed to get source, line, memory (PDB)");
            }
            break;
    }

    return RET_OK;
}



static int
get_source_path_line_memory_gdb (state_t *state)
{
    char *src_ptr,
         *dest_ptr,
         *prev_ptr;
    window_t *win;
    bool is_running;
    file_data_t *file_data;

    const char *key_threads  = "threads=[",
               *key_addr     = "addr=\"",
               *key_func     = "func=\"",
               *key_fullname = "fullname=\"",
               *key_line     = "line=\"";

    win = state->plugins[Src]->win;
    file_data = win->file_data;
    is_running = true;

    file_data->path_pos = 0;
    file_data->addr_pos = 0;
    file_data->func_pos = 0;

    // check if program running
    src_ptr = state->debugger->data_buffer;
    if (send_command_mp (state, "-thread-info\n") == RET_FAIL) {
        pfemr ("Failed to send source, line, memory (program running check) command (GDB)");
    }

        // get number of threads
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_threads);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_threads);
    } else {
        src_ptr = prev_ptr;
        *src_ptr = ']';
    }


    // if no threads or program not running
    if (*src_ptr == ']') {

        if (send_command_mp (state, "-file-list-exec-source-files\n") == RET_FAIL) {
            pfemr ("Failed to send source, line, memory (no threads) command (GDB)");
        }

        src_ptr = state->debugger->data_buffer;
        is_running = false;
    }

    // memory address
    file_data->addr_pos = 0;
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_addr);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_addr);
        while (*src_ptr != '\"') {
            cp_fchar (file_data, *src_ptr++, ADDR);
        }
    } else {
        src_ptr = prev_ptr;
    }
    cp_fchar (file_data, '\0', ADDR);

    // function
    file_data->func_pos = 0;
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_func);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_func);
        while (*src_ptr != '\"') {
            cp_fchar (file_data, *src_ptr++, FUNC);
        }
    } else {
        src_ptr = prev_ptr;
    }
    cp_fchar (file_data, '\0', FUNC);
    
    // absolute path
    src_ptr = strstr (src_ptr, key_fullname);
    dest_ptr = state->debugger->format_buffer;
    if (src_ptr != NULL) {
        src_ptr += strlen (key_fullname);
        while (*src_ptr != '\"') {
            *dest_ptr++ = *src_ptr++;
        }
        *dest_ptr = '\0';

        // path changed
        if (strcmp (state->debugger->format_buffer, win->file_data->path) != 0) {
            strncpy (win->file_data->path, state->debugger->format_buffer, FILE_PATH_LEN - 1);
            win->file_data->path_changed = true;
        } 

        // line number
        state->debugger->format_pos = 0;
        if (is_running) {
            src_ptr = strstr (src_ptr, key_line);
            src_ptr += strlen (key_line);
            while (*src_ptr != '\"') {
                cp_dchar (state->debugger, *src_ptr++, FORMAT_BUF);
            }
            win->file_data->line = atoi (state->debugger->format_buffer);
            state->debugger->format_buffer[0] = '\0';
        } else {
            win->file_data->line = 0;
        }
    }

    state->debugger->data_buffer[0] = '\0';

    return RET_OK;
}



static int
get_source_path_line_memory_pdb (state_t *state)
{
    char *src_ptr;
    window_t *win;
    file_data_t *file_data;

    const char *curr_path_symbol = "\n> ",
               *caret_str        = "<string>";

    win = state->plugins[Src]->win;
    file_data = win->file_data;
    src_ptr   = state->debugger->cli_buffer;

    file_data->path_pos = 0;
    file_data->func_pos = 0;

    if (send_command_mp (state, "where\n") == RET_FAIL) {
        pfemr ("Failed to send source path, line, memory command (PDB)");
    }

    src_ptr = strstr (src_ptr, curr_path_symbol);
    src_ptr += strlen (curr_path_symbol);

    // path
    state->debugger->format_pos = 0;
    while (*src_ptr != '(') {
        cp_dchar (state->debugger, *src_ptr++, FORMAT_BUF);
    }

    ++src_ptr;

    if (strcmp (state->debugger->format_buffer, caret_str) != 0) {

        if (strcmp (state->debugger->format_buffer, win->file_data->path) != 0) {
            strncpy (win->file_data->path, state->debugger->format_buffer, FILE_PATH_LEN - 1);
            win->file_data->path_changed = true;
        }

        // line number
        state->debugger->format_pos = 0;
        do {
            cp_dchar (state->debugger, *src_ptr++, FORMAT_BUF);
        } while (*src_ptr != ')');
        win->file_data->line = atoi (state->debugger->format_buffer);
    }

    state->debugger->cli_buffer[0] = '\0';
    state->debugger->format_buffer[0] = '\0';

    return RET_OK;
}
