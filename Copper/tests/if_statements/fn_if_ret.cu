a = { if ( true ) { ret(true) } else { ret(false) } }
assert(a())
a = { if ( false ) { ret(false) } else { ret(true) } }
assert(a())
