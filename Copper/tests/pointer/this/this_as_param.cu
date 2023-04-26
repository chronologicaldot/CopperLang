a = {
	this.b = true
	this.c = fn(p) { assert(p.b()) }
	this.c(this)
}
a()
