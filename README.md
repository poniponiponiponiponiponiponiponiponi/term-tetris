# Tetris
## About
My Tetris clone written in ncurses. Why yet another tetris? Every other
terminal tetris that I found and tested was somewhat lacking.
None of them implemented most of the machanics that define modern tetris -
things like seven bag randomization and lock delay.
I'm not a tetris pro myself so if something is wrong or missing let me know.
## TODO
    - Improving movement (use the keyboard device or something
    instead of the getch function),
    - settings for customization,
    - multiplayer.
## Controls
    - `left arrow` moves to left.
    - `right arrow` moves to right.
    - `down arrow` moves one block down.
    - `up arrow` or `x` clockwise rotation.
    - `z` counter clockwise rotation.
    - `c` hold currect block.
    - `space` hard drop block.
## Installation
git clone the repo and after that type `make` in the repo's directory.
This will create the tetris executable in the current working directory.
