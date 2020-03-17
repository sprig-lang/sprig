# Sprig
A light-weight programming language for minimalists.

## The Gist
Function definition syntax looks like:
```
foo a b = a + b
```
Constants can be defined as zero parameter functions:
```
pi = 3.1415
```
Side effects and overhead can be managed with interpolation expressions, which are evaluated before the activation of the function in which they appear.
```
bar = [foo 1 2], bar, bar, bar
```
In the above example `[foo 1 2]` is evaluated only once, despite the `bar` function being called multiple times.  Functions are activated at their point of definition.  Before this point, and within the same context as a definition, a function can be referenced but not called.

References allow for a limited degree of mutability.
```
r = [ref 'initial value']
r:set 'some other value'

"References shared between tasks can be synced with atomic operations
ref_val = [r:get]
r:set_if_equals rp new_val
```

Tasks allow for async and concurrency.
```
task = [execute \_(do_a_thing) nil]
result = task:result
```

Mirrors allow for reflection over objects.
```
mir = [mirror some_obj]
result = [mirror:invoke_if_exists 'some_fun' default_val]
```

Objects contain groups of functions, and are immutable themselves, though references can be used to add mutable state.
```
obj = {r = [ref nil]}
obj:r:set 123
```

Lambda expressions are syntax sugar for functional objects (objects with a single function named `f`).
```
fun = \a b(a + b)
fun:f 1 2
```

Cache expressions allow for manual caching of reusable values.
```
fibo n = [cache \n(n < 2 then n else fibo (n-1) + fibo (n-2))]:f n
```

Once expressions allow for expressions that are only evaluated once per location.
```
r = [once \(ref nil)]:get
```

There are two conditional operators: `then` and `else`.  The `then` operator returns its RHS if falsy, otherwise evaluating and returning the LHS.  The `else` expression returns the LHS if truthy, otherwise evaluating and returning the RHS.  The `else` operator has lower precedence than the `then` operator, so the two combined can be used where `if-else` statements/expressions would be in other languages.
```
cond1 then thing1 else cond2 then thing2 else thing3
```
These operators can also be used separately where `||` and `&&` might be in other languages.

The `catch` operator can be used with a thrown method call for more flexible control flow.
```
result = {return v = {result = v}, panic e = {error = e}} catch return! "Result"
```
