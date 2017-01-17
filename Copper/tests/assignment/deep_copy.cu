a = { this.b = true ret(this.b) }
c = a()
c.b = false
assert(a.b())
assert(not(c.b()))
