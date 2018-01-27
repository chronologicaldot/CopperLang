# Should print a no-function-in-container warning. #
a = { true }
b~a
a = { false }
assert( are_empty(b()) )
