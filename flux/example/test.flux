func add(a, b) {
  out a + b
}
func sub(a, b) {
  out a - b
}
func mult(a, b) {
  out a * b
}
func divid(a, b) {
 out a / b
}
func exponent(a, b) {
    result = 1
    i = 0
    while (i < b) {
        result = result * a
        i = i + 1
    }
    out result
}
func somatory(a) {
    out a * (a + 1) / 2
}
x = input('Which operation? (add, subtract, multiply, divide, exponent, somatory): ')
if (x == 'add') {
    a = input('Type number: ')
    b = input('Type number: ')
    add(a, b)
}
if (x == 'subtract') {
    a = input('Type number: ')
    b = input('Type number: ')
    sub(a, b)
}
if (x == 'multiply') {
    a = input('Type number: ')
    b = input('Type number: ')
    mult(a, b)
}
if (x == 'divide') {
    a = input('Type number: ')
    b = input('Type number: ')
    divid(a, b)
}
if (x == 'exponent') {
    a = input('Type number: ')
    b = input('Type number: ')
    exponent(a, b)
}
if (x == 'somatory') {
    a = input('Type number: ')
    somatory(a)
}