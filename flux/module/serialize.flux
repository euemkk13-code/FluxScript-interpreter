
--[[
FluxScript Serialization Module
Built with FluxScript v1.0.0.

Provides:
- dumps(value)
- loads(data)
- clone(value)
- isvalid(data)

Supported types:
- nil
- bool
- number
- string
- array
- dictionary

Uses json.flux as the serialization backend.

Note:
This version supports JSON-compatible values.
It does not serialize functions or cyclic references.
]]

get json

func dumps(value) {
    return json.stringify(value)
}

func loads(data) {
    assert type(data) == "string", "SerializeError: expected string"
    return json.parse(data)
}

func clone(value) {
    return loads(dumps(value))
}

func isvalid(data) {
    if type(data) != "string" {
        return false
    }

    try {
        loads(data)
        return true
    } catch {
        return false
    }
}
