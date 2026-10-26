
--[[
FluxScript Random Module
Uses time.now() for automatic seeding.
Park-Miller pseudo-random number generator.
]]

get time

seed = time.now() % 2147483647

if seed <= 0 {
    seed = 1
}

func setseed(value) {
    seed = int(value) % 2147483647

    if seed <= 0 {
        seed = 1
    }

    return seed
}

func next() {
    seed = (seed * 16807) % 2147483647
    return seed
}

func rand() {
    return next() / 2147483647
}

func randint(min, max) {
    if min > max {
        return nil
    }

    return min + int(random() * (max - min + 1))
}

func choice(array) {
    if len(array) == 0 {
        return nil
    }

    index = randint(0, len(array) - 1)
    return array[index]
}

func shuffle(array) {
    i = len(array) - 1

    while i > 0 {
        j = randint(0, i)

        temp = array[i]
        array[i] = array[j]
        array[j] = temp

        i -= 1
    }

    return array
}
