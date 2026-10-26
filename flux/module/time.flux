--[[
FluxScript time module
All clock values use UTC.
The native timestamp() has one-second precision.
]]

func now() {
    return timestamp()
}

func ms() {
    return timestamp() * 1000
}

func sec() {
    return timestamp() % 60
}

func min() {
    return (timestamp() // 60) % 60
}

func hrs() {
    return (timestamp() // 3600) % 24
}

func days() {
    return timestamp() // 86400
}