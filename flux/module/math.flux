func sq(x) {
    return x ^ 2
}
func cube(x) {
    return x ^ 3
}
func abs(x) {
    if x < 0 {
        return 0 - x
    }
    return x
}
func min(a, b) {
    if a < b {
        return a
    }
    return b
}
func max(a, b) {
    if a > b {
        return a
    }
    return b
}

func sqrt(x) {
    if x < 0 {
        return 0
    }
    if x == 0 {
        return 0
    }
    guess = x
    if x < 1 {
        guess = 1
    }
    i = 0
    while i < 30 {
        guess = (guess + x / guess) / 2
        i += 1
    }
    return guess
}
func fact(n) {
    if n < 0 {
        return 0
    }
    result = 1
    i = 2
    while i <= n {
        result *= i
        i += 1
    }
    return result
}
func gcd(a, b) {
    a = abs(a)
    b = abs(b)
    while b != 0 {
        temp = b
        b = a % b
        a = temp
    }
    return a
}
func lcm(a, b) {
    if a == 0 or b == 0 {
        return 0
    }
    return abs(a * b) / gcd(a, b)
}
func floor(x) {
    i = x // 1
    if x < 0 and x != i {
        return i - 1
    }
    return i
}
func ceil(x) {
    return 0 - floor(0 - x)
}
func round(x) {
    return floor(x + 0.5)
}
func sign(x) {
    if x > 0 {
        return 1
    }
    if x < 0 {
        return -1
    }
    return 0
}
