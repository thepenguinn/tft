# tft - tree for tables

This is yet another stupid idea I had. So basically `tft` is just the standard
GNU [tree](https://linux.die.net/man/1/tree) command but for lua tables. The
`tree` command will print the directory structure in a tree like format using
utf 8 characters. `tft` also does the same, but with a lua table. Put the lua
table in a file and pass it as the argument to `tft` and it will print the
table structure in a tree format.

# Building

First of all clone the repo and `cd` to the directory

```sh
git clone https://github.com/thepenguinn/tft
cd tft
```

You don't need anything fancy other than a c compiler, and `make` (guess what,
you don't even need `make`).

Run the command below if you have `gcc` and `make` installed, from the root
directory of this project.

```sh
make
```

Or you can build it with just the compiler, after all that `Makefile` is just a
wrapper for this command.

```sh
gcc tft.c -o tft
```

# Usage

Add a lua table to a file, say `hai.lua`. If you want, just use the below
snippet as the table.

```lua
tbl = {
    "something",
    lol = {
        "wow",
        "bye",
    },
    this = "is",
    "awesome",
}
```

Then run `tft` on that file, in this case its `hai.lua`

```sh
./tft hai.lua
```

This would output something like this, but with colors (Disclaimer: Currently
there is no way to suppress the colors).

```
tbl
├── 1 ─→ "something"
├── lol
│   ├── 1 ─→ "wow"
│   └── 2 ─→ "bye"
├── this ─→ "is"
└── 2 ─→ "awesome"
```

# Disclaimer

This thing will only work if:

- The file contains a single table and nothing else.
- And it is valid `lua`.
