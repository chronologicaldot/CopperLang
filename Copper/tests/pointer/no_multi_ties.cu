# Reassignment of a pointer should never tie multiple pointers. #
a = true
b 
c~b
c~a
assert( are_fn(b()) )
