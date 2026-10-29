--[[
    FluxScript math module
    From basic operations to high-level ones.
]]
get random
func chance(percent) {
    if percent <= 0 {
        return false
    }
    if percent >= 100 {
        return true
    }
    return random.randint(1, 1000000) <= percent * 10000
}
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
    return x ^ 0.5
}
func cbrt(x) {
    if x < 0 {
        return 0
    }
    if x == 0 {
        return 0
    }
    return x ^ (1 /3)
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
func sin(x) {
    result = x
    term = x
    i = 1
    for i(10) {
        term = -term * x * x / ((2*i) * (2*i+1))
        result += term
    }

    return result
}

func cos(x) {
    return sin(x + 3.1415926535 / 2)
}
func tan(x) {
    c = cos(x)
    if c == 0 {
        return 0 
    }
    return sin(x) / c
}