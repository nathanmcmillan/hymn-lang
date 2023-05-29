# Release 0.10.0

- Fixed tail call bug
- Fixed memory leak from functions
- New function `_stack` prints the current stack
- New function `_reference` prints a given object's reference counts
- Renamed global function `inspect` to `_source`
- Renamed global function `debug` to `_opcodes`
- Changed `let` keyword to `set`
- New `text` library function: `split`
- Changed echo on end of script to only occur for direct commands or REPL

# Release 0.9.0

- Added proper exception throwing to standard library functions
- Fixed bug in `pattern` library where ending `]` of class was included as part match
- Optimized removing POP instructions preceding a VOID return
- Fixed double echo in REPL when printing an exception
- Fixed dot syntax when using table keys that share a reserved keyword name
- Fixed bad stack management when optimizing incrementing a local
- Underscores allowed (again) in variable names
- Improved REPL line editing
- Added REPL support to save and load command history
- New `io` library function: `mkdir`
- New `text` library function: `last`
- New function `debug` prints a given function's byte code

# Release 0.8.0

- Hyphens now allowed in variable names
- Underscores no longer allowed in variable names
- Library `pattern` function `find` now returns start and end positions for groups
- Bug fixes and additional GCC / Clang warning fixes

# Release 0.7.0

- No exception thrown when defining or redefining a global variable without using `let`
- Refactored compile and runtime exception messages
- Numbers now allowed in variable names
- New instructions `OP_NEW_ARRAY` and `OP_NEW_TABLE` for performance
- Renamed global variables
  - `__globals` -> `GLOBALS`
  - `__paths` -> `PATHS`
  - `__imports__` -> `IMPORTS`
- Renamed functions
  - `string` -> `str`
- Renamed keywords
  - `function` -> `func`
- String formatting now uses `"${}"` style syntax
- New syntax `->` for self calls on table functions
- New standard library functions
  - pattern: `get`, `find`, `match`, and `replace`
- Support for binary `0b`, hexadecimal `0x`, and scientific notation numbers
- New function `inspect` prints a given function's source code

# Release 0.6.2

- Various optimized OP codes for common instruction sequences
- New REPL with external editor support
- Changed comments to use `#` symbol
- Now using C style brackets `{` and `}` instead of `begin` and `end` keywords
- Quoted strings can now be used for table keys
- Tail call optimizations
- Optional type declarations of the form `name: type`
  - Currently has no affect on compiling
- Shared C libarary loading `.so` / `.dll` files
- New separate executable for formatting scripts
- Removed OP code optimizations from JavaScript version to keep it simple
- Added new command line arguments for `help`, `version`, and `interactive` support
- New standard library functions
  - io: `readlines`, `write`, `append`, and `move`
  - os: `exec`
  - text: `leftstrip`, `rightsrip`, and `join`
  - json: `save` and `parse`

# Release 0.5.1

- New optimized OP code `OP_GET_GLOBAL_PROPERTY` for global variables
- New typedefs `HymnInt` and `HymnFloat`
- Cleaned up error messages
- Refactored function names and exposed more as public
- Significantly expanded standard library

# Release 0.4.0

- Added `exists` function. Returns true if a key exists in a table
- Added `echo` statements for printing values
- Changed `print` to take in a boolean to switch between standard out and error. New lines are no longer included
- Added `__globals` for self referencing global variables
- Fixed various memory leaks

# Release 0.3.0

- For loop syntax updated to follow the form `for i = 0, i < X, i += 1`
- Iterators updated to use the form `for K, V in I`
- Adjacent string literals are now combined at compile time
- Strings now support runtime formatting using brackets. For instance `"Literal with {V}."` will substitute `{V}` with the value of variable `V`
