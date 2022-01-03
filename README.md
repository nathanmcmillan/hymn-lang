# Hymn Scripting Language

A small byte-code interpreted language with a focus on simplicity. [Visit the website!](https://hymn-lang.org)

```
-- import external scripts
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

## Fix Me

1. Need public function for setting table properties

## Planned

1. Intern non-string constants (scan for existing values)
1. A failed malloc, calloc, realloc should return a compile or runtime error if possible
1. Bash-like string formatting `"I am a ${variable}"` when using double quotes
1. C-like adjacent string literals are merged together
1. Consistent, descriptive error messages. Use the form `[Expression Name]: Expected/Missing [X], but was [Y]`
1. Keywords `yield` `resume` `start` for coroutines
1. Keywords `async` `await` for asynchronous calls
1. Keywords `#if` `#else` `#elif` `#define` `#end` for macros
1. Add `finally` keyword for after try blocks
1. Add `open` keyword that automatically tries to call a `close` function at the end of a block
1. Instead of `OP_PRINT` etc, should they be calls to C functions. Otherwise they can't be used as function pointers
1. Standard library written in Hymn

## Progress

| Implementation | Status        | Description         |
| -------------- | ------------- | ------------------- |
| C              | Version 0.2.1 | New for loop syntax |
| JavaScript     | Version 0.2.0 | New for loop syntax |
| Go             | Not Started   |                     |
| Java           | Not Started   |                     |

## Performance

1. Use virtual registers with an infinite stack, with 32 bit instructions, rather than using a stack
1. Use value pooling to reduce allocations and freeing
1. Delay dereferencing to the end of subroutines and analyze where reference counting can be ignored
1. Tagged pointers or NaN boxing
1. Tail call optimizations. Any time a function ends with another function call, the stack can be re-used
1. Insert into table collision linked lists using sorted order, so subsequent operations can be quicker
1. Refactor string related handling to reduce copying

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
