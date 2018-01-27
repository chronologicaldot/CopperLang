a = {
	assert( are_bool(this.c()) )
	assert( this.c() )
}
b ~ a
b.c = true
a()
