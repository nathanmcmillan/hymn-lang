# Hymn Scripting Language

A small byte-code interpreted language with a focus on simplicity. [Visit the website!](https://hymn-lang.org)

```
-- import additional scripts
use "math"

-- tables hold key value pairs
function new_node(value)
  return { value: value, next: none }
end

function node_add(list, value)
  let node = list
  while true
    if node.next == none break end
    node = node.next
  end
  node.next = new_node(value)
end

-- objects are passed by reference
let list = new_node("hello")
node_add(list, "world")

-- print statements will show all nested values in an object
echo list
```

# Why use Hymn

1. You want reference counting memory management. Memory is released deterministically, as soon as possible. There are no random garbage collection pauses in Hymn.
1. You feel programming paradigms like classes and interfaces add unnecessary complexity. Hymn only has string, array, and table objects.
1. You don't need namespaces. The Hymn `use` statement imports all variables and functions globally.
1. You're weary keeping up with ever evolving programming languages. Hymn is small and will stay small. There will not be significant changes to the core language and built-in functions.
1. You want a scripting language where indices start at 0 and the not equals operator is `!=`

# Development

## Principles

1. Portable C11 code that passes with `-Wall` `-Wextra` `-Werror` `-pedantic` flags
1. Small implementation under 7,000 total lines of C code
1. Reference counting memory management
1. Easy to learn syntax
1. No closures
1. No namespaces
1. No classes
1. No breaking changes after version 1.0

## To Do

1. `OP_TABLE` & `OP_ARRAY` to avoid if statement in `OP_CONSTANT` instruction
1. Optimizations
   - Registers to avoid push / pop / references
   - If we have two number only instructions in a row
   - We can take out the type check for the second instruction
   - We know an exception will be thrown from the first instruction if the types are not allowed
1. Tail call optimizations. Any time a function ends with another function call, the stack can be re-used
1. Consistent, descriptive error messages
1. Add special `@` array for vardic functions `@[0] == parameter[0]`
1. Add `finally` keyword for try blocks
1. Refactor string related handling to reduce copying
   - HymnString and HymnObjectString are separate objects
   - Can HymnObjectString have it's own `char*`?
1. Insert into table collision linked lists using sorted order, so subsequent operations can be quicker
1. A failed malloc, calloc, realloc should return a compile or runtime error if possible

## Progress

| Implementation | Status        | Description                      |
| -------------- | ------------- | -------------------------------- |
| C              | Version 0.5.1 | Get Global Property Optimization |
| JavaScript     | Version 0.5   | Anonmyous functions              |
| Go             | Not Started   |                                  |
| Java           | Not Started   |                                  |

## Notes

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

## Performance

AMD Ryzen 5 1600 6 core 3.2 GHz - Windows 10 using Ubuntu WSL

| Test   | Factors | Fib   | List  | Loop  | Primes |
| ------ | ------- | ----- | ----- | ----- | ------ |
| Hymn   | 1.65s   | 1.55s | 2.64s | 1.25s | 2.78s  |
| Lua    | 1.14s   | 1.16s | 2.05s | 0.77s | 0.81s  |
| Python | 4.17s   | 2.27s | 2.67s | 2.00s | 2.59s  |

# Compiling

## Linux + GCC

### Debug

```
$ gcc src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -o hymn -lm
```

### Testing

```
$ gcc test/*.c src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -DHYMN_TESTING -Isrc -o hymntest -lm
```

### Release

```
$ gcc src/*.c -std=c11 -O3 -s -DNDEBUG -o hymn -lm
```

## Windows + MSVC

### Debug

```
> cl src/*.c /W4 /WX /wd4996 /link /out:HYMN.exe
```

### Testing

```
> cl test/*.c src/*.c /Isrc /W4 /WX /wd4996 -DHYMN_TESTING /link /out:HYMNTEST.exe
```

### Release

```
> cl src/*.c /O2 /DNDEBUG /link /out:HYMN.exe
```

## Windows + Clang

### Debug

```
> clang src/*.c -Isrc -Wall -Wextra -Werror -pedantic -std=c11 -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -o HYMN.exe
```

### Testing

```
> clang test/*.c src/*.c -Isrc -Wall -Wextra -Werror -pedantic -std=c11 -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -DHYMN_TESTING -o HYMNTEST.exe
```

### Release

```
> clang -O2 src/*.c -Isrc -Wno-deprecated-declarations -std=c11 -o HYMN.exe
```

## Node

### Testing

```
$ node js/test.js
```

# Profiling on Linux

```
$ gcc src/*.c -O0 -pg -o profile -lm
$ ./profile [FILE].hm
$ gprof profile gmon.out | less
```

```
$ valgrind --tool=callgrind ./hymn [FILE].hm
$ callgrind_annotate --auto=yes callgrind.out.* | less
```

# Memory Leaks on Linux

```
$ valgrind --leak-check=full --show-leak-kinds=all -s ./hymn [FILE].hm
```
