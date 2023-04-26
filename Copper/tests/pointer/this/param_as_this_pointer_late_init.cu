a = {
	this.b = fn(parent~this) {
		assert( this.parent.c() )
	}
}
a()
a.c = true
a.b()
