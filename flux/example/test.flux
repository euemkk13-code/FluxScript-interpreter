
out "========== FLUXSCRIPT 0.7.0 =========="
out "TEST 1: DICTIONARY"
player = {"name": "Alex", "hp": 100}
out player["name"]
player["hp"] = 75
out player["hp"]
out "TEST 2: DICTIONARY METHODS"
out player.keys
out player.values
out player.len
player.remove "hp"
out player
out "TEST 3: ARRAY METHODS"
numbers = [5, 2, 9, 1, 3]
out numbers.sum
out numbers.min
out numbers.max
out numbers.avg
numbers.sort
out numbers
numbers.rev
out numbers
out "TEST 4: STRING METHODS"
name = "  FluxScript  "
out name.trim
out name.upper
out name.lower
out name.has "Script"
out name.replace "Flux", "Super"
out "TEST 5: F-STRING"
version = "0.7.0"
out f"FluxScript version {version}"
out f"2 + 3 = {2 + 3}"
out f"Escaped braces: {{hello}}"
out "TEST 6: SLICING"
items = [10, 20, 30, 40, 50]
out items[1:4]
out items[::-1]
out items[::2]
out "TEST 7: TERNARY"
age = 18
status = "adult" if age >= 18 else "minor"
out status
out "TEST 8: DEFAULT PARAMETERS"
func greet(name, message = "Hello") {
    return f"{message}, {name}!"
}
out greet("Alex")
out greet("Alex", "Welcome")
out "TEST 9: TRY CATCH"
try {
    out 10 / 0
} catch(error) {
    out "Error caught!"
    out error
}
out "TEST 10: FILE OPERATIONS"
saveFile "test_070.txt", "Hello FluxScript!"
out readFile("test_070.txt")
addFile "test_070.txt", " v0.7.0"
out readFile("test_070.txt")
out fileExists("test_070.txt")
deleteFile "test_070.txt"
out fileExists("test_070.txt")
out "========== TEST FINISHED =========="
