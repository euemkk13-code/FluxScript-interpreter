--[[
FluxScript String Module
Version: 0.6.0
String utility functions.
]]
func repeat(text, times) {
    result = ""
    i = 0
    while i < times {
        result += text
        i += 1
    }
    return result
}
func reverse(text) {
    result = ""
    i = len(text) - 1
    while i >= 0 {
        result += text[i]
        i -= 1
    }
    return result
}