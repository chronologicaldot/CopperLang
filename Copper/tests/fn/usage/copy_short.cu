a = { this.b = true }
c = a
c()
assert(c.b())
