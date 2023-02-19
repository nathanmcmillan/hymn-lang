# Compiling

## Linux + GCC

### Debug

```
$ gcc src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wpadded -Wundef -Wpointer-arith -Wunreachable-code -Wuninitialized -Winit-self -Wmissing-include-dirs -Wswitch-default -Wunused -Wunused-parameter -Wunused-variable -Wunused-value -Wshadow -Wconversion -Wcast-qual -Wcast-align -Wwrite-strings -Wlogical-op -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wvla -Woverlength-strings -Wstrict-overflow=5 -g -o hymn -lm -ldl
```

Include `-rdynamic` for dynamic library support

### Testing

```
$ gcc test/*.c src/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -DHYMN_TESTING -Isrc -o hymntest -lm -ldl
```

### Benchmark

```
gcc test/*.c src/*.c -std=c11 -O3 -s -DNDEBUG -DHYMN_NO_CLI -DHYMN_NO_TEST -DHYMN_BENCHMARK -Isrc -o hymnbenchmark -lm -ldl
```

### Release

```
$ gcc src/*.c -std=c11 -O3 -s -DNDEBUG -o hymn -lm -ldl
```

### Formatter

```
$ gcc format/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -g -o hymnfmt
```

### Dynamic Library

```
$ gcc -Isrc -fpic -c test/dlib/dlib.c
$ gcc -shared -lc -o dlib.so dlib.o
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

### Formatter

```
> cl format/*.c /W4 /WX /wd4996 /link /out:HYMNFMT.exe
```

### Dynamic Library

```
> cl src/hymn.c /W4 /WX /wd4996 /LD
> cl /LD /Isrc hymn.lib test\\dlib\\dlib.c
```

## Windows + Clang

### Debug

```
> clang src/*.c -Isrc -std=c11 -Wall -Wextra -Weverything -Werror -pedantic -Wduplicate-decl-specifier -Wdeprecated-register -Wempty-body -Wnull-dereference -Wparentheses -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -Wno-float-equal -Wno-bad-function-cast -Wno-switch-enum -Wno-covered-switch-default -Wno-format-nonliteral -o HYMN.exe
```

### Testing

```
> clang test/*.c src/*.c -Isrc -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -DHYMN_TESTING -o HYMNTEST.exe
```

### Release

```
> clang -O2 src/*.c -Isrc -std=c11 -Wno-deprecated-declarations -o HYMN.exe
```

### Formatter

```
> clang format/*.c -std=c11 -Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-deprecated-declarations -Wno-gnu-zero-variadic-macro-arguments -Wno-language-extension-token -o HYMNFMT.exe
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
