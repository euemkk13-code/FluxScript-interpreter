
--[[
FluxScript Storage Module
Built with FluxScript v1.0.0.

Persistent key-value storage.

Functions:
- open(path)
- set(db, key, value)
- get(db, key)
- has(db, key)
- remove(db, key)
- keys(db)
- values(db)
- clear(db)
- save(db)
- close(db)

Requires serialize.flux.
]]

get serialize

func _check(db) {
    assert db["closed"] == false, "StorageError: database is closed"
}

func open(path) {
    assert type(path) == "string", "StorageError: path must be a string"

    data = {}

    if fileexists(path) {
        content = readfile(path)
        data = serialize.loads(content)

        assert type(data) == "dict", "StorageError: invalid database format"
    }

    return {
        "path": path,
        "data": data,
        "closed": false
    }
}

func set(db, key, value) {
    _check(db)
    assert type(key) == "string", "StorageError: key must be a string"

    db["data"][key] = value
    return true
}

func get(db, key) {
    _check(db)

    if key in db["data"] {
        return db["data"][key]
    }

    return nil
}

func has(db, key) {
    _check(db)
    return key in db["data"]
}

func remove(db, key) {
    _check(db)

    if key in db["data"] {
        db["data"].remove key
        return true
    }

    return false
}

func keys(db) {
    _check(db)
    return db["data"].keys
}

func values(db) {
    _check(db)
    return db["data"].values
}

func clear(db) {
    _check(db)
    db["data"] = {}
    return true
}

func save(db) {
    _check(db)

    content = serialize.dumps(db["data"])
    writefile(db["path"], content)

    return true
}

func close(db) {
    _check(db)

    save(db)
    db["closed"] = true

    return true
}
