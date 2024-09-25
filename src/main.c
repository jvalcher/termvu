//
// TODO: re-render layout on terminal screen size change
// TODO: replace strncpy with memcpy
// TODO: put parse_configuration_file() through its paces; add more error handling
// TODO: add user error messages to pfem... functions; dev flag for current message formatting
//

#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <errno.h>

#include "data.h"
#include "utilities.h"
#include "parse_config_file.h"
#include "render_layout.h"
#include "start_debugger.h"
#include "run_plugin.h"
#include "persist_data.h"
#include "update_window_data/_update_window_data.h"
#include "plugins.h"

static int  initial_configure   (int, char*[], state_t*);
static void exit_signal_handler (int sig_num);



int
main (int   argc,
      char *argv[]) 
{
    int        key;
    state_t    state;
    debugger_t debugger;
    state.debugger = &debugger;

    if (initial_configure (argc, argv, &state) == RET_FAIL) {
        pfeme ("Initial configuration failed\n\n");
    }

    if (parse_config_file (&state) == RET_FAIL) {
        pfeme ("Failed to parse configuration file\n\n");
    }

    if (render_layout (FIRST_LAYOUT, &state) == RET_FAIL) {
        pfeme ("Failed to render \"%s\" layout\n\n", FIRST_LAYOUT);
    }

    if (start_debugger (&state) == RET_FAIL) {
        pfeme ("Failed to start debugger");
    }

    if (update_windows (&state, 2, Src, Asm) == RET_FAIL) {
        pfeme ("Failed to update windows");
    }

    if (get_persisted_data (&state) == RET_FAIL) {
        pfeme ("Failed to get persisted data");
    }

    while (debugger.running) {
        key = getch ();
        if (run_plugin (state.plugin_key_index[key], &state) == RET_FAIL) {
            pfeme ("Failed to run plugin");
        }
    }

    if (persist_data (&state) == RET_FAIL) {
        pfeme ("Failed to persist data");
    }

    clean_up ();

    return EXIT_SUCCESS;
}



/*
    Initial configuration
    ---------

    - CLI flag arguments

        -h
            - Prints usage instructions

        -c  Configuration file path
            - See README.md for more information
            - default: CONFIG_FILE
            - Outputs to state->config_path

        -p  Data persistence file path
            - Persists watchpoints, breakpoints
            - default: PERSIST_FILE
            - Outputs to state->data_path

        -d
            - Wait for debugger to attach

    - Set signals

    - Initialize Ncurses
*/
static int
initial_configure (int   argc,
                   char *argv[],
                   state_t *state)
{
    int opt;
    extern char *optarg;
    bool debugging_mode;
    FILE *fp;

    //
    // CLI arguments
    //
    state->config_path[0] = '\0';
    state->data_path[0]   = '\0';
    debugging_mode = false;
    char *optstring = "hdc:p:";

    while ((opt = getopt (argc, argv, optstring)) != -1) {
        switch (opt) {

            // help
            case 'h':
                printf (

                "\n"
                "Usage: \n"
                "\n"
                "   $ termfu\n"
                "\n"
                "       Run in same directory as a %s configuration file\n"
                "       Data persisted to ./%s\n"
                "\n"
                "   $ termfu [OPTION...]\n"
                "\n"
                "       -c CONFIG_FILE    Use this configuration file\n"
                "       -p PERSIST_FILE   Persist sessions with this file\n"
                "\n"
                "       -d                Start in debug mode\n"
                "\n",

                CONFIG_FILE, PERSIST_FILE);

                exit (EXIT_SUCCESS);

            // configuration file
            case 'c':
                strncpy (state->config_path, optarg, CONFIG_PATH_LEN - 1);
                break;

            // data persist file
            case 'p':
                strncpy (state->data_path, optarg, DATA_PATH_LEN - 1);
                break;

            // debugger mode
            case 'd':
                debugging_mode = true;
                break;

            default:
                fprintf (stderr,

                "\n"
                "Run with -h flag to see usage instructions.\n"
                "\n");

                exit (EXIT_FAILURE);
        }
    }


    // debugging mode
    if (debugging_mode) {

        // write debugged termfu PID to DEBUG_PID_FILE
        if ((fp = fopen (DEBUG_PID_FILE, "w")) == NULL) {
            pfem ("fopen error: %s", strerror (errno));
            pemr ("Unable to open debug PID file \"%s\"\n", DEBUG_PID_FILE);
        }
        fprintf (fp, "%ld", (long) getpid());
        fclose (fp);

        // print message
        printf (
            "\n"
            "Process ID:    \033[0;36m%ld\033[0m (%s)\n"
            "\n"
            "Connect to this process with debugger\n"
            "\n"
            "    $ make conn_proc_<debugger>\n"
            "    - Set breakpoint\n"
            "    - Continue\n"
            "\n"
            "Press any key to continue...\n"
            "\n",
            (long) getpid (), DEBUG_PID_FILE);

        // wait...
        getchar ();
    }


    //
    // Signals
    //
    signal (SIGINT, exit_signal_handler);    // Ctrl-C


    //
    // Ncurses
    //
    initscr();

    if (has_colors()) {
        start_color();
        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);           // RED_BLACK
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);       // GREEN_BLACK
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);     // YELLOW_BLACK
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);         // BLUE_BLACK
        init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);   // MAGENTA_BLACK
        init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);         // CYAN_BLACK
        init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);       // WHITE_BLACK
        init_pair(WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);         // WHITE_BLUE
    } else {
        pfemr ("Terminal doesn't support colors\n");
    }

    cbreak();
    noecho();
    curs_set (0);
    set_escdelay (0);

    return RET_OK;
}



static void
exit_signal_handler (int sig_num)
{
    // TODO: persist data on SIGINT
    (void) sig_num;
    clean_up ();
    fprintf (stderr, "termfu exited (SIGINT)\n");
    exit (EXIT_FAILURE);
}

