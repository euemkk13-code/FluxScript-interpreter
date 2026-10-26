get string
get json
out string.repeat("Hi", 3)
out string.reverse("Hello")
out string.repeat("FluxScript! ", 2)

out "========== JSON TEST =========="
out "TEST 1: STRINGIFY"
player = {
    "name": "Alex",
    "hp": 100,
    "level": 5,
    "alive": true,
    "items": ["sword", "shield", "potion"]
}
text = json.stringify(player)
out text
out "TEST 2: PARSE"
restored = json.parse(text)
out restored["name"]
out restored["hp"]
out restored["alive"]
out "TEST 3: SAVE"
json.save("player.json", player)
out "File saved!"
out "TEST 4: LOAD"
loaded = json.load("player.json")
out loaded["name"]
out loaded["level"]
out loaded["items"]
out "TEST 5: ARRAY"
numbers = [10, 20, 30, 40]
arrayText = json.stringify(numbers)
out arrayText
out json.parse(arrayText)
out "TEST 6: PRIMITIVES"
out json.stringify(true)
out json.stringify(false)
out json.stringify(nil)
out json.stringify(3.14159)
out json.parse("123")
out json.parse("true")
out json.parse("null")
out "TEST 7: NESTED OBJECT"
world = {
    "player": player,
    "settings": {
        "volume": 80,
        "fullscreen": false
    }
}
nestedText = json.stringify(world)
out nestedText
nested = json.parse(nestedText)
out nested["settings"]["volume"]
out "========== TEST FINISHED =========="
