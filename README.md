# Hymn Scripting Language

A small byte-code interpreted language with a focus on simplicty. [Visit the website!](https://hymn-lang.org)

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
print list
```

# Why use Hymn

1. You want reference counting memory management. Memory is released deterministically, as soon as possible. There are no random garbage collection pauses in Hymn.
1. You feel programming paradigms like classes and interfaces add unnecessary complexity. Hymn only has string, array, and table objects.
1. You don't need namespaces. Hymn `use` statements import all variables and functions globally.
1. You're weary keeping up with ever evolving programming languages. Hymn is small and will stay small. There will not be significant changes to the core language and built-in functions.
1. You want a scripting language where indices start at 0 and the not equals operator is `!=`

# Compiling

## GCC

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

## MSVC

### Debug

```
> cl src/*.c /W4 /WX /wd4996 /link /out:hymn.exe
```

### Testing

```
> cl test/*.c src/*.c /Isrc /W4 /WX /wd4996 -DHYMN_TESTING /link /out:hymntest.exe
```

### Release

```
> cl src/*.c /O2 /DNDEBUG /link /out:hymn.exe
```

## Profiling on Linux

```
$ gcc src/*.c -O0 -pg -o profile -lm
$ ./profile [FILE].hm
$ gprof profile gmon.out | less
```

```
$ valgrind --tool=callgrind ./hymn [FILE].hm
$ callgrind_annotate --auto=yes callgrind.out.* | less
```

# Development

## Principles

1. Portable C11 code that passes with `-Wall` `-Wextra` `-Werror` `-pedantic` flags
1. Small implementation under 6,000 total lines of C code
1. Reference counting memory management
1. No closures
1. No namespaces
1. No classes
1. No new langauage features
1. Long term compatibility
1. Emphasis on writing simple and readable scripts
1. Emphasis on one way to do things right

## Planned

1. More constants than 256
1. Intern non-string constants
1. A failed malloc, calloc, etc should return a compile or runtime error if possible
1. Bash-like string formatting `"I am a ${variable}"` when using double quotes
1. Consistent, descriptive error messages. Use the form `[Expression Name]: Expected/Missing [X], but was [Y]`
1. Standard library written in Hymn
1. Keywords `yield` `resume` `start` for coroutines
1. Keywords `#if` `#else` `#define` `#end` for macros
1. Instead of `OP_PRINT` etc, should they be calls to C functions
1. Remove `pass`
1. Remove `do` maybe

## Progress

| Implementation | Status      |
| -------------- | ----------- |
| C              | Version 0.1 |
| JavaScript     | Version 0.1 |
| Go             | Not Started |
| Java           | Not Started |

## Performance

1. Use virtual registers with an infinite stack, with 32 bit instructions, rather than using a stack
1. Use value pooling to reduce allocations and freeing
1. Delay dereferencing to the end of subroutines and analyze where references counting can be ignored
1. Tagged pointers or NaN boxing
1. Tail call optimizations. Anytime a function ends with another function call, the stack can be re-used
1. Insert into table collision linked list in sorted order

## Notes

1. Using a macro READ_BYTE instead of an inline functions slightly improves performance
1. Using an instruction pointer rather than index significantly improves performance
1. Using computed goto statements instead of a big switch significantly impproves performance. But it is not ISO C
