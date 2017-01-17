b = true
a = fn(p) { p = {} }
a(b)
assert(b())
