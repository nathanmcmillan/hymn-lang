# Release 0.3.0

- For loop syntax was updated to follow the form `for i = 0, i < X, i += 1`
- Iterators were updated to use the form `for K, V in I`
- Adjacent string literals are now combined at compile time
- Strings now support runtime formatting using brackets. For instance `"Literal with {V}."` will substitute `{V}` with the value of variable `V`
