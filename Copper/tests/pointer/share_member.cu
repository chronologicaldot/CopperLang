a = {
	assert( is_bool(this.c()) )
	assert( this.c() )
}
b ~ a
b.c = true
a()
