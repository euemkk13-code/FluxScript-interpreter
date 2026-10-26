
func escapeString(value) {
    result = "\""
    for ch in value {
        if ch == "\"" {
            result += "\\\""
        } elif ch == "\\" {
            result += "\\\\"
        } elif ch == "\n" {
            result += "\\n"
        } elif ch == "\r" {
            result += "\\r"
        } elif ch == "\t" {
            result += "\\t"
        } else {
            result += ch
        }
    }
    return result + "\""
}
func stringify(value) {
    kind = type(value)
    if kind == "string" {
        return escapeString(value)
    } elif kind == "number" or kind == "bool" {
        return str(value)
    } elif kind == "nil" {
        return "null"
    } elif kind == "array" {
        result = "["
        first = true
        for item in value {
            if not first {
                result += ","
            }
            result += stringify(item)
            first = false
        }
        return result + "]"
    } elif kind == "dict" {
        result = "{"
        first = true
        keys = value.keys
        for key in keys {
            if not first {
                result += ","
            }
            result += escapeString(key) + ":" + stringify(value[key])
            first = false
        }
        return result + "}"
    }
    return "null"
}
func skipWhitespace(source, position) {
    while position < len(source) {
        ch = source[position]
        if ch != " " and ch != "\n" and ch != "\r" and ch != "\t" {
            break
        }
        position += 1
    }
    return position
}
func parseString(source, position) {
    if position >= len(source) or source[position] != "\"" {
        return [false, "", position]
    }
    position += 1
    result = ""
    while position < len(source) {
        ch = source[position]
        position += 1
        if ch == "\"" {
            return [true, result, position]
        }
        if ch == "\\" {
            if position >= len(source) {
                return [false, "", position]
            }
            ch = source[position]
            position += 1
            if ch == "n" {
                result += "\n"
            } elif ch == "r" {
                result += "\r"
            } elif ch == "t" {
                result += "\t"
            } elif ch == "\"" {
                result += "\""
            } elif ch == "\\" {
                result += "\\"
            } elif ch == "/" {
                result += "/"
            } else {
                return [false, "", position]
            }
        } else {
            if ch == "\n" or ch == "\r" or ch == "\t" {
                return [false, "", position]
            }
            result += ch
        }
    }
    return [false, "", position]
}
func isDigit(ch) {
    return ch in "0123456789"
}
func parseNumber(source, position) {
    start = position
    if source[position] == "-" {
        position += 1
    }
    if position >= len(source) {
        return [false, nil, position]
    }
    if source[position] == "0" {
        position += 1
    } else {
        if not source[position] in "123456789" {
            return [false, nil, position]
        }
        while position < len(source) {
            if not isDigit(source[position]) {
                break
            }
            position += 1
        }
    }
    if position < len(source) {
        if source[position] == "." {
            position += 1
            if position >= len(source) {
                return [false, nil, position]
            }
            if not isDigit(source[position]) {
                return [false, nil, position]
            }
            while position < len(source) {
                if not isDigit(source[position]) {
                    break
                }
                position += 1
            }
        }
    }
    if position < len(source) {
        if source[position] == "e" or source[position] == "E" {
            position += 1
            if position < len(source) {
                if source[position] == "+" or source[position] == "-" {
                    position += 1
                }
            }
            if position >= len(source) {
                return [false, nil, position]
            }
            if not isDigit(source[position]) {
                return [false, nil, position]
            }
            while position < len(source) {
                if not isDigit(source[position]) {
                    break
                }
                position += 1
            }
        }
    }
    return [true, float(source[start:position]), position]
}
func parseValue(source, position) {
    position = skipWhitespace(source, position)
    if position >= len(source) {
        return [false, nil, position]
    }
    ch = source[position]
    if ch == "\"" {
        return parseString(source, position)
    }
    if ch == "[" {
        position += 1
        items = []
        position = skipWhitespace(source, position)
        if position < len(source) {
            if source[position] == "]" {
                return [true, items, position + 1]
            }
        }
        while position < len(source) {
            item = parseValue(source, position)
            if not item[0] {
                return item
            }
            items.push item[1]
            position = skipWhitespace(source, item[2])
            if position >= len(source) {
                return [false, nil, position]
            }
            if source[position] == "]" {
                return [true, items, position + 1]
            }
            if source[position] != "," {
                return [false, nil, position]
            }
            position += 1
        }
        return [false, nil, position]
    }
    if ch == "{" {
        position += 1
        object = {}
        position = skipWhitespace(source, position)
        if position < len(source) {
            if source[position] == "}" {
                return [true, object, position + 1]
            }
        }
        while position < len(source) {
            position = skipWhitespace(source, position)
            key = parseString(source, position)
            if not key[0] {
                return [false, nil, position]
            }
            position = skipWhitespace(source, key[2])
            if position >= len(source) {
                return [false, nil, position]
            }
            if source[position] != ":" {
                return [false, nil, position]
            }
            value = parseValue(source, position + 1)
            if not value[0] {
                return value
            }
            object[key[1]] = value[1]
            position = skipWhitespace(source, value[2])
            if position >= len(source) {
                return [false, nil, position]
            }
            if source[position] == "}" {
                return [true, object, position + 1]
            }
            if source[position] != "," {
                return [false, nil, position]
            }
            position += 1
        }
        return [false, nil, position]
    }
    if source[position:position + 4] == "true" {
        return [true, true, position + 4]
    }
    if source[position:position + 5] == "false" {
        return [true, false, position + 5]
    }
    if source[position:position + 4] == "null" {
        return [true, nil, position + 4]
    }
    if ch == "-" or isDigit(ch) {
        return parseNumber(source, position)
    }
    return [false, nil, position]
}
func parse(source) {
    parsed = parseValue(source, 0)
    if not parsed[0] {
        return nil
    }
    if skipWhitespace(source, parsed[2]) != len(source) {
        return nil
    }
    return parsed[1]
}
func save(path, value) {
    saveFile(path, stringify(value))
}
func load(path) {
    return parse(readFile(path))
}
