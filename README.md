# slip

A lisp interpreter implemented in C

```bash
$ ./run.sh
Slip version 0.0.0.0
Press ctrl+c to exit
slip> + 2 2
4
slip> if (== 2 2) {True} {False}
True
slip> def {hoge} 2
()
slip> def {add} (\ {x y} {+ x y})
()
slip> add hoge 2
4
slip> (\ {arg} {- hoge arg}) 2
0
slip> let {fib} (\ {a b n} {if (== n 0) {a} {fib (+ a b) a (- n 1)}})

```

## Build

```bash
$ make
# Binary file `slip` should be generated in local directory
```

## Implemented features

- Integer Operation
  - add `+`
  - subtract `-`
  - mutiply `*`
  - divide `/`
- Boolean
  - `True`/`False`
  - Equality `==`
- Functions
  - lambda `\ {args} {body}`
  - Currying by default: `(\ {x y} {+ x y}) 2` -> `(\ {y} {+ x y})` (`x` is already bound)
- Variables
  - define globally `def {x} value`
- Macro
  - Not evaluated until passed to the `eval` function
  - `{` contents `}`
  - Can be used as list structure as well
  - `head` `{hoge fuga piyo}` -> `hoge`
  - `tail` `{hoge fuga piyo}` -> `{fuga piyo}`
  - `eval` `{func fuga piyo}` -> `func fuga piyo`
  - `list hoge fuga piyo` -> `{hoge fuga piyo}`
- S-expression
  - `()` pretty much what you'd expect from a paren
  - ex: `+ (+ 2 2) 2` -> `6`
