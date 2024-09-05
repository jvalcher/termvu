#include <unistd.h>

#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"
#include "choose_layout.h"



void
run_plugin (int      plugin_index,
            state_t *state)
{
    switch (plugin_index) {

        // commands
        case Con:
        case Fin:
        case Kil:
        case Nxt:
        case Run:
        case Stp:
        case Unt: 
        case Qut:
            send_debugger_command (plugin_index, state); 
            break;
                  
        // windows
        case Asm:
        case Brk:
        case Dbg:
        case LcV:
        case Prg:
        case Reg:
        case Src:
        case Wat: 
            if (state->plugins[plugin_index]->has_window) {
                select_window (plugin_index, state);
            }
            break;

        // other
        case Lay: 
            choose_layout (state);
            break;
    }
}


