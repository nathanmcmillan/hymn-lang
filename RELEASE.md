# Release 0.6.2

- Various optimized OP codes for common instruction sequences
- New REPL with external editor support
- Changed comments to use `#` symbol
- Now using C style brackets `{` and `}` instead of `begin` and `end` keywords
- Tail call optimizations
- Optional type declarations of the form `name: type`
  - Currently has no affect on compiling
- Shared C libarary loading `.so` / `.dll` files
- New separate executable for formatting scripts
- Removed OP code optimizations from JavaScript version to keep it simple
- Added new command line arguments for `help`, `version`, and `interactive` support
- New standard library functions
  - IO `readlines`, `write`, `append`, and `move`
  - OS `exec`
  - Text `leftstrip`, `rightsrip`, and `join`
  - New JSON library

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

- For loop syntax was updated to follow the form `for i = 0, i < X, i += 1`
- Iterators were updated to use the form `for K, V in I`
- Adjacent string literals are now combined at compile time
- Strings now support runtime formatting using brackets. For instance `"Literal with {V}."` will substitute `{V}` with the value of variable `V`
