# Hymn Scripting Language

A small byte-code interpreted language with a focus on simplicty. [Visit the website!](https://hymn-lang.org)

## Goals

1. Portable C11 code that passes with `-Wall` `-Wextra` `-Werror` `-pedantic` flags
1. Small implementation under 4,000 semicolons
1. Reference counting memory management
1. No closures
1. No namespaces
1. No classes
1. No new langauage features
1. Long term compatibility
1. Emphasis on writing simple and readable scripts
1. Emphasis on one way to do things right

## Development

1. More constants than 256
1. Intern non-string constants
1. A failed malloc, calloc, etc should return a compile or runtime error if possible
1. Multi-line strings without different syntax
1. Bash-like double quote string formatting `I am a ${variable}`
1. Consistent, descriptive error messages. Use the form `[Expression Name]: Expected/Missing [X], but was [Y]`

| Implementation | Status      |
| -------------- | ----------- |
| C              | Version 0.1 |
| JavaScript     | Version 0.1 |
| Rust           | Started     |
| Go             | Not Started |
| Java           | Not Started |

## Long Term Goals

1. Large standard library written in Hymn
