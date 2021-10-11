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
1. You feel programming paradigms like classes and interfaces add unnecessary complexity. Hymn only has string, list, and table objects.
1. You don't need namespaces. Hymn `use` statements import all variables and functions globally.
1. You're weary keeping up with ever evolving programming languages. Hymn is small and will stay small. There will not be significant changes to the core language and built-in functions.
1. You want a scripting language where indices start at 0 and the not equals operator is `!=`

# Compiling

## Linux with GCC

`$ gcc src/*.c src/*.h -o hymn -Isrc/include -lm`

## Windows with MSVC

`> cl src/*.c /I src/include /link /out:hymn.exe`

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
1. Multi-line strings without different syntax
1. Bash-like double quote string formatting `I am a ${variable}`
1. Consistent, descriptive error messages. Use the form `[Expression Name]: Expected/Missing [X], but was [Y]`
1. Large standard library written in Hymn

## Progress

| Implementation | Status      |
| -------------- | ----------- |
| C              | Version 0.1 |
| JavaScript     | Version 0.1 |
| Rust           | Started     |
| Go             | Not Started |
| Java           | Not Started |
