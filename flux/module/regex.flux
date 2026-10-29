
--[[
FluxScript Regex Module.
Built whit FluxScript v1.0.0.
Supports:
- Literals and escaped characters
- Character classes and ranges
- Dot, ^, $
- \d, \D, \w, \W, \s, \S
- Capturing groups
- Nested groups
- Alternation
- Greedy and lazy quantifiers
- *, +, ?, {n}, {n,}, {n,m}
- search, match, fullmatch, findall
- replace, split, groups, group
]]

func _node(kind, data, children) {
    return {"kind": kind, "data": data, "children": children}
}

func _isDigit(ch) {
    return ch >= "0" and ch <= "9"
}

func _isWord(ch) {
    return (ch >= "a" and ch <= "z") or (ch >= "A" and ch <= "Z") or _isDigit(ch) or ch == "_"
}

func _isSpace(ch) {
    return ch == " " or ch == "\t" or ch == "\n" or ch == "\r"
}

func _classHas(spec, ch) {
    neg = false
    pos = 0
    if len(spec) > 0 and spec[0] == "^" {
        neg = true
        pos = 1
    }
    found = false
    while pos < len(spec) {
        if pos + 2 < len(spec) and spec[pos + 1] == "-" {
            if ch >= spec[pos] and ch <= spec[pos + 2] {
                found = true
            }
            pos += 3
        } else {
            if ch == spec[pos] {
                found = true
            }
            pos += 1
        }
    }
    if neg {
        return not found
    }
    return found
}
func _readNumber(ctx) {
    pattern = ctx["pattern"]
    pos = ctx["pos"]
    start = pos
    value = 0
    digits = "0123456789"
    while pos < len(pattern) and _isDigit(pattern[pos]) {
        digit = 0
        i = 0
        while i < 10 {
            if pattern[pos] == digits[i] {
                digit = i
                break
            }
            i += 1
        }
        value = value * 10 + digit
        pos += 1
    }
    assert pos > start, "RegexError: expected number"
    ctx["pos"] = pos
    return value
}
func _parseAtom(ctx) {
    pattern = ctx["pattern"]
    pos = ctx["pos"]
    assert pos < len(pattern), "RegexError: expected atom"
    ch = pattern[pos]
    if ch == "(" {
        ctx["pos"] = pos + 1
        ctx["count"] += 1
        groupId = ctx["count"]
        inner = _parseAlt(ctx)
        assert ctx["pos"] < len(pattern) and pattern[ctx["pos"]] == ")", "RegexError: unclosed group"
        ctx["pos"] += 1
        return _node("group", groupId, [inner])
    }
    if ch == "[" {
        pos += 1
        start = pos
        while pos < len(pattern) and pattern[pos] != "]" {
            if pattern[pos] == "\\" and pos + 1 < len(pattern) {
                pos += 1
            }
            pos += 1
        }
        assert pos < len(pattern), "RegexError: unclosed character class"
        ctx["pos"] = pos + 1
        return _node("class", pattern[start:pos], [])
    }
    if ch == "\\" {
        assert pos + 1 < len(pattern), "RegexError: trailing backslash"
        code = pattern[pos + 1]
        ctx["pos"] = pos + 2
        if code == "d" or code == "D" or code == "w" or code == "W" or code == "s" or code == "S" {
            return _node("special", code, [])
        }
        if code == "n" {
            return _node("literal", "\n", [])
        }
        if code == "t" {
            return _node("literal", "\t", [])
        }
        if code == "r" {
            return _node("literal", "\r", [])
        }
        return _node("literal", code, [])
    }
    ctx["pos"] = pos + 1
    if ch == "." {
        return _node("dot", "", [])
    }
    if ch == "^" {
        return _node("begin", "", [])
    }
    if ch == "$" {
        return _node("end", "", [])
    }
    assert ch != "*" and ch != "+" and ch != "?" and ch != "}" and ch != "]" and ch != "{", "RegexError: unexpected operator"
    return _node("literal", ch, [])
}

func _parseRepeat(ctx) {
    atom = _parseAtom(ctx)
    pattern = ctx["pattern"]
    pos = ctx["pos"]
    if pos >= len(pattern) {
        return atom
    }
    ch = pattern[pos]
    minimum = 1
    maximum = 1
    hasQuantifier = true
    if ch == "*" {
        minimum = 0
        maximum = -1
        pos += 1
    } elif ch == "+" {
        minimum = 1
        maximum = -1
        pos += 1
    } elif ch == "?" {
        minimum = 0
        maximum = 1
        pos += 1
    } elif ch == "{" {
        ctx["pos"] = pos + 1
        minimum = _readNumber(ctx)
        pos = ctx["pos"]
        maximum = minimum
        if pos < len(pattern) {
            if pattern[pos] == "," {
                ctx["pos"] = pos + 1
                pos = ctx["pos"]
                if pos < len(pattern) and pattern[pos] == "}" {
                    maximum = -1
                } else {
                    maximum = _readNumber(ctx)
                    pos = ctx["pos"]
                }
            }
        }
        assert pos < len(pattern) and pattern[pos] == "}", "RegexError: unclosed repetition"
        assert maximum == -1 or maximum >= minimum, "RegexError: invalid repetition range"
        pos += 1
    } else {
        hasQuantifier = false
    }
    if not hasQuantifier {
        return atom
    }
    assert atom["kind"] != "begin" and atom["kind"] != "end", "RegexError: anchor cannot be repeated"
    lazy = false
    if pos < len(pattern) and pattern[pos] == "?" {
        lazy = true
        pos += 1
    }
    ctx["pos"] = pos
    return _node("repeat", {"min": minimum, "max": maximum, "lazy": lazy}, [atom])
}

func _parseSeq(ctx) {
    pattern = ctx["pattern"]
    children = []
    while ctx["pos"] < len(pattern) {
        ch = pattern[ctx["pos"]]
        if ch == ")" or ch == "|" {
            break
        }
        children.add _parseRepeat(ctx)
    }
    return _node("seq", "", children)
}

func _parseAlt(ctx) {
    children = [_parseSeq(ctx)]
    pattern = ctx["pattern"]
    while ctx["pos"] < len(pattern) and pattern[ctx["pos"]] == "|" {
        ctx["pos"] += 1
        children.add _parseSeq(ctx)
    }
    if len(children) == 1 {
        return children[0]
    }
    return _node("alt", "", children)
}

func _compile(pattern) {
    ctx = {"pattern": pattern, "pos": 0, "count": 0}
    root = _parseAlt(ctx)
    assert ctx["pos"] == len(pattern), "RegexError: unexpected closing parenthesis"
    return {"root": root, "count": ctx["count"]}
}

func _accept(node, ch) {
    kind = node["kind"]
    data = node["data"]
    if kind == "literal" {
        return ch == data
    }
    if kind == "dot" {
        return ch != "\n"
    }
    if kind == "class" {
        return _classHas(data, ch)
    }
    if kind == "special" {
        if data == "d" {
            return _isDigit(ch)
        }
        if data == "D" {
            return not _isDigit(ch)
        }
        if data == "w" {
            return _isWord(ch)
        }
        if data == "W" {
            return not _isWord(ch)
        }
        if data == "s" {
            return _isSpace(ch)
        }
        if data == "S" {
            return not _isSpace(ch)
        }
    }
    return false
}

func _copyCaps(caps) {
    result = []
    for item in caps {
        result.add item
    }
    return result
}

func _state(pos, caps) {
    return {"pos": pos, "caps": caps}
}

func _repeatWalk(node, text, state, count, depth) {
    result = []
    info = node["data"]
    minimum = info["min"]
    maximum = info["max"]
    lazy = info["lazy"]
    if count >= minimum and lazy {
        result.add state
    }
    if maximum == -1 or count < maximum {
        nextStates = _eval(node["children"][0], text, state, depth + 1)
        for nextState in nextStates {
            if nextState["pos"] != state["pos"] {
                more = _repeatWalk(node, text, nextState, count + 1, depth + 1)
                for item in more {
                    result.add item
                }
            } elif count + 1 >= minimum {
                result.add nextState
            }
        }
    }
    if count >= minimum and not lazy {
        result.add state
    }
    return result
}

func _eval(node, text, state, depth) {
    assert depth < 512, "RegexError: recursion limit"
    kind = node["kind"]
    pos = state["pos"]
    if kind == "literal" or kind == "dot" or kind == "class" or kind == "special" {
        if pos < len(text) and _accept(node, text[pos]) {
            return [_state(pos + 1, state["caps"])]
        }
        return []
    }
    if kind == "begin" {
        if pos == 0 {
            return [state]
        }
        return []
    }
    if kind == "end" {
        if pos == len(text) {
            return [state]
        }
        return []
    }
    if kind == "seq" {
        states = [state]
        for child in node["children"] {
            nextStates = []
            for current in states {
                matches = _eval(child, text, current, depth + 1)
                for matchState in matches {
                    nextStates.add matchState
                }
            }
            states = nextStates
            if len(states) == 0 {
                break
            }
        }
        return states
    }
    if kind == "alt" {
        result = []
        for child in node["children"] {
            matches = _eval(child, text, state, depth + 1)
            for matchState in matches {
                result.add matchState
            }
        }
        return result
    }
    if kind == "group" {
        result = []
        matches = _eval(node["children"][0], text, state, depth + 1)
        for matchState in matches {
            caps = _copyCaps(matchState["caps"])
            caps[node["data"]] = text[pos:matchState["pos"]]
            result.add _state(matchState["pos"], caps)
        }
        return result
    }
    if kind == "repeat" {
        return _repeatWalk(node, text, state, 0, depth + 1)
    }
    return []
}

func _initialCaps(count) {
    caps = []
    i = 0
    while i <= count {
        caps.add nil
        i += 1
    }
    return caps
}

func _run(compiled, text, start, anchored, requireEnd) {
    pos = start
    while pos <= len(text) {
        caps = _initialCaps(compiled["count"])
        states = _eval(compiled["root"], text, _state(pos, caps), 0)
        for state in states {
            if not requireEnd or state["pos"] == len(text) {
                finalCaps = _copyCaps(state["caps"])
                finalCaps[0] = text[pos:state["pos"]]
                return {"start": pos, "end": state["pos"], "groups": finalCaps}
            }
        }
        if anchored {
            break
        }
        pos += 1
    }
    return nil
}

func search(pattern, text) {
    compiled = _compile(pattern)
    hit = _run(compiled, text, 0, false, false)
    if hit == nil {
        return nil
    }
    return hit["groups"][0]
}

func match(pattern, text) {
    compiled = _compile(pattern)
    hit = _run(compiled, text, 0, true, false)
    if hit == nil {
        return nil
    }
    return hit["groups"][0]
}

func fullmatch(pattern, text) {
    compiled = _compile(pattern)
    hit = _run(compiled, text, 0, true, true)
    if hit == nil {
        return nil
    }
    return hit["groups"][0]
}

func findall(pattern, text) {
    compiled = _compile(pattern)
    result = []
    pos = 0
    while pos <= len(text) {
        hit = _run(compiled, text, pos, false, false)
        if hit == nil {
            break
        }
        result.add hit["groups"][0]
        if hit["end"] == hit["start"] {
            pos = hit["end"] + 1
        } else {
            pos = hit["end"]
        }
    }
    return result
}

func replace(pattern, replacement, text) {
    compiled = _compile(pattern)
    output = ""
    pos = 0
    while pos <= len(text) {
        hit = _run(compiled, text, pos, false, false)
        if hit == nil {
            return output + text[pos:]
        }
        output += text[pos:hit["start"]] + replacement
        if hit["end"] == hit["start"] {
            if hit["end"] < len(text) {
                output += text[hit["end"]]
            }
            pos = hit["end"] + 1
        } else {
            pos = hit["end"]
        }
    }
    return output
}

func split(pattern, text) {
    compiled = _compile(pattern)
    result = []
    pos = 0
    while pos <= len(text) {
        hit = _run(compiled, text, pos, false, false)
        if hit == nil {
            result.add text[pos:]
            return result
        }
        result.add text[pos:hit["start"]]
        if hit["end"] == hit["start"] {
            pos = hit["end"] + 1
        } else {
            pos = hit["end"]
        }
    }
    result.add ""
    return result
}

func groups(pattern, text) {
    compiled = _compile(pattern)
    hit = _run(compiled, text, 0, false, false)
    if hit == nil {
        return nil
    }
    return hit["groups"]
}

func group(pattern, text, index=0) {
    found = groups(pattern, text)
    if found == nil {
        return nil
    }
    assert index >= 0 and index < len(found), "RegexError: invalid group index"
    return found[index]
}
