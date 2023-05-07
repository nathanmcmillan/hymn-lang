# Bugs

1. Use `int` for size and capcity type data including arrays and strings
   - Add `INT_MAX` safety checks
1. Memory leaks when file not found
1. Fix and thoroughly test OP code optimizations
   - Can we assign the original byte index as a unique identifier
   - And only at the very end, iterate through and match them, then calculate the final difference?
1. Finish JSON module
   - Handle scientific notation
1. Finish dynamic library support
   - Export everything
   - Build as a library

# Features

1. UTF8 encode and decode strings and codes with `\u0000`
1. Self referencing local functions
1. Add special `@` array for vardic functions `@[0] == parameter[0]`
1. Add `finally` keyword for try blocks
1. Refactor string related handling to reduce copying
   - HymnString and HymnObjectString are separate objects
   - Can HymnObjectString have it's own `char*`?
1. Optimize string handling to skip interning for special cases
1. Insert into table collision linked lists in sorted order
1. Macro system
1. Compile time constant evaluation
1. Transpile to C code and use a C compiler to generate a dynamic library at runtime
1. Cache byte code in a `.hymn_cache` directory

# Notes

1. Registers greatly improve performance by avoiding additional push, pop, and referencing
   - But requires quite a lot more complexity to implement
1. Using a macro READ_BYTE instead of an inline functions improves performance
1. Using an instruction pointer rather than an index improves performance
1. Using computed goto instead of a switch **does not** always improve performance
1. Instructions using a bit mask flag to control local / global / constant information is **terrible** for performance
   - Likely the extra work to extract information from the bit mask is too costly
   - Or the branching instructions were impossible to predict
1. A single monolithic source file **significantly** improves performance compared to multiple source and header files
   - Likely due to inlining and better compiler information
   - No code organization!
1. Namespaces require significant complexity but they can be simulated using a table with anonymous functions
1. Deferred reference counting doesn't work
   - Intermediate values such as a concatenated string are only briefly present on the stack
1. Lua loops with cached increment and bounds in special instructions is a major performance boost
