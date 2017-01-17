a = {
	this.b = fn(parent~this) {
		assert( this.parent.c())
	}
	this.b()
}
a.c = true
a()
