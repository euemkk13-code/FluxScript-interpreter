get math
out "=== FluxScript v0.4.0 TEST ==="

out "TEST 1: Math operators"
out 10 % 3
out 2 ^ 5
out 7 // 2
out 5 + 3 * 2
out (5 + 3) * 2

out "TEST 2: Assignment operators"
x = 10
x += 5
out x

x -= 3
out x

x *= 2
out x

x /= 4
out x

x %= 4
out x

x ^= 3
out x

out "TEST 3: Arrays"
numbers = [10, 20, 30, 40, 50]
out numbers[0]
out numbers[2]
out numbers[4]

out "TEST 4: For loop"
for i(5) {
    out i
}

out "TEST 5: If without parentheses"
age = 18

if age >= 18 {
    out "Adult"
} else {
    out "Minor"
}

out "TEST 6: While without parentheses"
counter = 0

while counter < 3 {
    out counter
    counter += 1
}

out "TEST 7: Functions"
func square(n) {
    return n * n
}

out square(6)

out "TEST 8: Logic"
a = 10
b = 20

if a < b and b > 15 {
    out "AND works"
}

if a > b or b == 20 {
    out "OR works"
}
out 'TEST 8: module math'
out math.square(5)
out math.cube(3)
out math.remainder(10, 3)

out "=== ALL TESTS FINISHED ==="
