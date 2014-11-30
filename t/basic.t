# This is a comment

return_10
    : 10

return_str
    : "foo"

isEqual a b
  ? = a b
    : 1
  : 0

loop value
  ? = value 10
    : value
  + value 1

object a b
  method1 a
    : b
  method2
    + a b
    : a
  : 0

append string
  + string "bar"
  : string
