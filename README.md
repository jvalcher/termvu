
<h1 align="center">termfu</h1>
<p align="center">An easy-to-use, multi-language terminal debugger that allows users to create and switch between custom layouts</p>
<br>
<p align="center"><img margin-left="auto" src="./misc/layout1.png" width="700px"></p>
<br>


## Support

| Debugger   | Languages |
|   :----:   | -----     |
| [GDB](https://sourceware.org/gdb/) | C, C++, D, Go, Objective-C, Fortran, OpenCL C, Pascal, Rust, assembly, Modula-2, Ada |
| [PDB](https://docs.python.org/3/library/pdb.html) | Python |
<br>

## Install from source

### Dependencies

The Makefile uses `ncurses6`.
```
sudo apt-get install libncurses-dev gdb pdb make
```
### Build
```
make
```
<br>

Copy `termfu` to the desired executable directory, e.g. `/usr/bin`. 
<br><br>


## Usage

- Without the `-f` flag, the program must be run in the same directory as a `.termfu` configuration file.
```
termfu
```
- Otherwise, choose the desired configuration file path.
```
termfu -f config/.termfu_01
```
- Configure the debugger command and layout(s) inside the configuration file.
- Breakpoints and watchpoints are persisted in `.termfu_data`. 
- Select a window and scroll through the data using arrow or `hjkl` keys.
- Switch layouts with `(l)ayouts`.
- Send custom debugger commands with `pro(m)pt`.
<br><br>


## Configuration
Each three-character, case-sensitive plugin code corresponds to a specific action or window. These codes are mapped to keys, which are used as shortcuts and to create custom layouts. 
<br>

### Plugins

__Header Commands__
<br>

| Code    | Description |
| :-----: | ------ |
| Con     | Continue |
| Fin     | Finish |
| Kil     | Kill |
| Lay     | Choose layout |
| Nxt     | Next |
| Prm     | Debugger prompt |
| Qut     | Quit |
| Run     | Run program (reload binary if needed) |
| Stp     | Step |
| Trg     | Target remote server (e.g. gdbserver) |
| Unt     | Until |
<br>

__Windows__
<br>

| Code    | Description      | GDB                | PDB                |
| :-----: | -----            | :-----:            | :------:           |
| Asm     | Assembly code    | :heavy_check_mark: |                    |   
| Brk     | Breakpoints      | :heavy_check_mark: | :heavy_check_mark: |
| Dbg     | Debugger output  | :heavy_check_mark: | :heavy_check_mark: |
| LcV     | Local variables  | :heavy_check_mark: |                    |
| Prg     | Program output   | :heavy_check_mark: |                    |
| Reg     | Registers        | :heavy_check_mark: |                    |
| Src     | Source file      | :heavy_check_mark: | :heavy_check_mark: |
| Stk     | Stack            | :heavy_check_mark: |                    |
| Wat     | Watchpoints      | :heavy_check_mark: | :heavy_check_mark: |
<br>


### Configuration sections

- Adding parentheses around a character in a `<(t)itle>` changes the character's color.
- Only newline `# comments` are supported, not inline.

| Section   | Description |
| :----:    |  :----: |
| command | Debugger command |
| plugins | \<plugin code\> : \<key binding\> : \<title\> |
| layout  | \[ layout : <title> \] <br>`>h` : header commands,  `>w` : windows |
<br>

### Commands

| Debugger | Command |
| :-----:  | ------  |
| GDB      | `gdb --interpreter=mi ...` |
| PDB      | `python -m pdb ...` |
<br>

### Example `.termfu` configuration


```
[ command ]

gdb --interpreter=mi misc/hello

[ plugins ]

# header commands
Prm : m : pro(m)pt
Trg : t : (t)arget
Lay : l : (l)ayouts
Qut : q : (q)uit
Run : r : (r)un
Nxt : n : (n)ext
Stp : s : (s)tep
Con : c : (c)ontinue
Unt : u : (u)ntil
Fin : f : (f)inish
Kil : k : (k)ill

# windows
Asm : a : (a)ssembly
Brk : e : br(e)akpoints
Dbg : d : (d)ebug out
LcV : v : local (v)ars
Prg : p : (p)rogram out
Reg : g : re(g)isters
Src : o : s(o)urce file
Stk : T : s(T)ack
Wat : w : (w)atch


[ layout : Main ]

>h
mtlq
rnscufk

>w
eeeooooo
wwwooooo
vvvooooo
TTpppddd

[ layout : Assembly / Registers ]

>h
mtlq
rnscufk

>w
oag
oag
oag
wpT
```
<br>


### Resulting layouts

<img src='./misc/layout1.png' height='400px'>
<img src='./misc/layout2.png' height='400px'>
<br>

## Releases

- __v1.0__ - _09-17-2024_ - Initial release providing basic GDB, PDB functionality
<br>

## Contributing

### General Guidelines
- For any significant contributions outside of minor patches, open an issue first.
- Bug fixes, optimizations, new debugger implementations, and new plugins are welcome.
- Blank space and comment PRs have a high probability of being closed.
- Use existing code conventions.

### Developer notes
- Run the `make configs` script to create all needed configuration files in `misc/` for running the included sample binaries or scripts (`make run_dev_gdb`, etc.) and for debugging. Feel free to edit the `scripts/create_configs` script to customize layouts, change the target binary, add plugins, etc. However, do __NOT__ include your customized script in a PR.
- All watchpoints and breakpoints will be persisted in `_data` files alongside their relevant configuration files in `misc/`.
- Run `make todo` to print all source file tags, such as `TODO`, `FIX`, etc.
- The `make debug` script starts a `tmux`-based `GDB` TUI debugging session. See the comments in `scripts/gdb_debug_tui` for usage.
- The `make server` and `make target` scripts allow `termfu` to debug itself. See the comments in `scripts/gdb_debug_server` for usage. __Note__: stepping through the program using these scripts is currently slow. There is an `OPTIMIZE` source tag for speeding this up.
- The `logd()` function in `src/utilities.h` allows for `printf()`-style debugging when running `ncurses` by outputting to `debug.out`.
- It is recommended to create a shortcut for refreshing your terminal screen, as `ncurses` will make a mess of it when not shut down properly. For example, add `bind r respawn-pane -k` to `~/.tmux.conf` to refresh your `tmux` pane with `Ctrl+b` then `r`.


### Scripts

`./Makefile`
<br>
| Command             | Description |
| --------            | -------     |
| `make`              | Build production binary |
| `make dev`	        | Build development binary|
| `make devf`         | Build development binary, print formatted error messages |
| `make configs`      | Create all sample runs, debugging configuration files|
| `make run_dev_gdb`  | Run development binary with sample gdb binary |
| `make run_dev_pdb`  | Run development binary with sample pdb script |
| `make todo`         | Print source code tags  (`TODO`, `FIXME`, etc.) |
| `make tui_gdb`      | Start tmux GDB TUI debugging session  (see `scripts/gdb_debug`) |
| `make tui_pdb`      | Start tmux GDB TUI debugging session  (see `scripts/pdb_debug`) |
|	`make server`       | Start termfu_dev with gdbserver  (see `scripts/gdb_server`) |
| `make target`       | Start termfu_dev and target the gdbserver started  by `make server` |
<br>

`./tests/Makefile`
<br>
| Command           | Description |
| --------          | -------     |
| `make t=test1.c`  | Build, run `test1.c` |
| `make debug`      | Debug most recent build |

<br><br>

