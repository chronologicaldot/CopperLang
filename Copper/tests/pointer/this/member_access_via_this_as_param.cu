a = {
	this.b = true
	this.c = fn(d~this) {
		assert( this.d.b() )
	}
	this.c()
}
a()
