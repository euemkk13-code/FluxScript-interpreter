get string
get json
get console
out string.repeat("Hi", 3)
out string.reverse("Hello")
out string.repeat("FluxScript! ", 2)

--[[
FluxScript v0.8.0
Console Module Test
]]

console.title("FluxScript v0.8.0 - Test")
console.clear()
wait(1)
console.color("cyan")
wait(1)
out "=================================="
out "      FLUXSCRIPT v0.8.0"
out "=================================="
wait(1)
console.reset()
wait(1)
out ""
out "TEST 1 - COLORS"
wait(1)
console.color("green")
out "Green text!"
wait(0.3)
console.color("yellow")
out "Yellow text!"
wait(0.3)
console.color("red")
out "Red text!"
wait(0.3)
console.color("blue")
out "Blue text!"
wait(0.3)
console.reset()
wait(0.3)
out ""
out "TEST 2 - BACKGROUND"
wait(1)
console.background("blue")
wait(0.3)
console.color("white")
out "White text on blue background!"
console.reset()
wait(1)
out ""
out "TEST 3 - CONSOLE INFO"
wait(1)
pos = console.position()
dimensions = console.size()
wait(1)
out f"Cursor position: {pos}"
out f"Console size: {dimensions}"
wait(1)
out ""
out "TEST 4 - PROGRESS BAR"
wait(1)
console.progress(0, 30)
wait(0.4)
console.progress(25, 30)
wait(0.4)
console.progress(50, 30)
wait(0.4)
console.progress(75, 30)
wait(0.4)
console.progress(100, 30)

out ""
out "TEST 5 - BOX"
wait(0.4)
console.box("FluxScript v0.8.0!")
wait(0.4)
out ""
out "TEST 6 - BEEP"
wait(0.4)
console.beep(800, 200)
wait(0.4)
out ""
out "TEST 7 - KEYBOARD"
wait(0.4)
out "Press any key..."
key = console.readKey()
out f"Pressed key: {key}"

out ""
out "TEST 8 - MENU"

options = ["Start", "Settings", "Exit"]
selected = console.menu(options)

out f"Selected option: {selected}"
wait(1)
out ""
out "TEST 9 - CURSOR"
wait(1)
console.hideCursor()
out "Cursor hidden!"
wait(1)
console.showCursor()
out "Cursor visible!"

out ""
console.color("green")
out "=================================="
out "       TEST COMPLETED!"
out "=================================="
console.reset()
wait(1)


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
