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
print(list)
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

## Ideas

1. Tail call optimizations. Any time a function ends with another function call, the stack can be re-used
1. Consistent, descriptive error messages. Use the form `[Expression Name]: Expected/Missing [X], but was [Y]`
1. Add `finally` keyword for after try blocks
1. Refactor string related handling to reduce copying
1. Insert into table collision linked lists using sorted order, so subsequent operations can be quicker
1. A failed malloc, calloc, realloc should return a compile or runtime error if possible
1. Standard library written in Hymn
1. Delay dereferencing to the end of subroutines and analyze where reference counting can be ignored

## Progress

| Implementation | Status      | Description       |
| -------------- | ----------- | ----------------- |
| C              | Version 0.3 | String formatting |
| JavaScript     | Version 0.3 | String formatting |
| Go             | Not Started |                   |
| Java           | Not Started |                   |

## Notes

1. Using a macro READ_BYTE instead of an inline functions improves performance
1. Using an instruction pointer rather than index improves performance
1. Using computed goto instead of a switch **_does not_** always improve performance

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
