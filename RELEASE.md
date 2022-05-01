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
