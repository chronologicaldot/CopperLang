a = {
	this.b = true
	this.c = fn(d~this.b) {
		assert( this.d() )
	}
	this.c()
}
a()
