# FluxScript-interpreter

FluxScript is an indie programming language created on October 24, 2026. It is designed to be simple, fast, and easy to learn.

I hope that in future versions, FluxScript will be able to create games and applications and become popular. Not for fame, but so people can code something with less struggle.

**Current version: 1.0.0**

FluxScript currently supports variables, printing, user input, arithmetic and logical operators, conditionals, ternary expressions, loops, arrays, dictionaries, string manipulation, functions with default parameters, lambda expressions, named arguments, class inheritance, complex numbers, functional programming with `map()`, `filter()`, and `reduce()`, assertions, aliases, hierarchical packages, modules, regular expressions, serialization, persistent key-value storage, file operations, type conversion, error handling, built-in utility functions, raw strings, escape sequences, native console operations, and Windows GUI applications with interactive controls, customizable properties, mouse and keyboard events, image support, dialogs, and non-blocking timers.

## HOW TO RUN FLUXSCRIPT FILES

### Step 1 - Open CMD

Open the Start menu, type `cmd`, and press Enter.

<img width="1366" height="768" alt="Open CMD" src="https://github.com/user-attachments/assets/fc6376eb-93f0-47fe-8d25-ffa87c839f74" />

### Step 2 - Navigate to the FluxScript folder

Type:

    cd "C:\Example\flux"

Replace `C:\Example\flux` with the actual path to your FluxScript folder.

Use quotation marks if the path contains spaces.

<img width="1366" height="768" alt="Navigate to FluxScript folder" src="https://github.com/user-attachments/assets/e93d929f-1a4a-4d63-909f-31cc16775601" />

### Step 3 - Run a FluxScript file

Type:

    flux.exe example\test.flux

Replace `test.flux` with the name of your file and `example` with its folder.

If the file is in the same folder as `flux.exe`, type:

    flux.exe test.flux

You CANNOT use `.flx` extension.

<img width="1366" height="768" alt="Run FluxScript file" src="https://github.com/user-attachments/assets/bb274f80-00e0-4dd9-a3e7-8f7c963033a5" />

Done! Your FluxScript program should now run.

## HOW TO INTEGRATE FLUXSCRIPT WITH VS CODE

You can make Visual Studio Code recognize FluxScript files and highlight their syntax.

### Step 1 - Locate the extension folder

Find the `fluxscript-syntax` folder included with FluxScript.

This folder contains the syntax highlighting and language configuration files.

### Step 2 - Install the extension

Press `Win + R` and type:

    %USERPROFILE%\.vscode\extensions

Copy the entire `fluxscript-syntax` folder into this directory.

The folder structure should look like this:

    .vscode/

    └── extensions/

        └── fluxscript-syntax/

            ├── package.json

            ├── language-configuration.json

            └── syntaxes/

                └── fluxscript.tmLanguage.json

### Step 3 - Restart VS Code

Close and reopen Visual Studio Code.

Open a `.flux` file.

VS Code should now recognize FluxScript and apply syntax highlighting.

### Step 4 - Run FluxScript with F5 (optional)

You can configure VS Code to run FluxScript files by pressing F5.

First, create a `.vscode` folder inside your FluxScript project.

Inside it, create a file named `tasks.json`:

    {

        "version": "2.0.0",

        "tasks": [

            {

                "label": "Run FluxScript",

                "type": "shell",

                "command": "${workspaceFolder}\\flux.exe",

                "args": [

                    "${file}"

                ],

                "options": {

                    "cwd": "${workspaceFolder}"

                },

                "problemMatcher": [],

                "presentation": {

                    "reveal": "always",

                    "panel": "dedicated",

                    "clear": true

                }

            }

        ]

    }

Next, press `Ctrl + Shift + P` and search for `Preferences: Open Keyboard Shortcuts (JSON)`.

Add this shortcut inside the existing JSON array:

    {

        "key": "f5",

        "command": "workbench.action.tasks.runTask",

        "args": "Run FluxScript",

        "when": "editorTextFocus && (resourceExtname == .flx || resourceExtname == .flux)"

    }

If you already have other shortcuts, separate the entries with commas.

Open a FluxScript file and press F5 to run it.

**Note:** This runs the program using `flux.exe`. It does not provide debugging or breakpoints.

# FLUXSCRIPT v1.0.0 TUTORIAL

### 1. Printing text and values

FluxScript uses `out` to print text and values, similar to Python's `print()`.

You don't need parentheses or semicolons.

Example:

    out "Hello, World!"

    out 'Hello again!'

    out 123

    out 5 + 5

Both single and double quotation marks are supported for strings.

You can also concatenate strings using `+`.

Example:

    out "Hello" + ", World!"

Output:

    Hello, World!

### 2. Comments

FluxScript supports single-line and multiline comments.

#### Single-line comments

Use `//` for single-line comments.

Example:

    // This is a comment

    out "Hello"

#### Multiline comments

Use `--[[` to start a multiline comment and `]]` to end it.

Example:

    --[[

    This is a multiline comment.

    You can write multiple lines here.

    Hello!

    1

    2

    3

    ]]

**Important:** Since `//` is also used for floor division, it is recommended to put single-line comments on their own lines.

### 3. Variables

You can create variables using `=`.

There is no need to declare them with `let`, `var`, or a type.

Example:

    x = 10

    name = "FluxScript"

    pi = 3.14159

    enabled = true

    out x

    out name

    out pi

    out enabled

Variables can store numbers, strings, booleans, arrays, and dictionaries.

FluxScript uses dynamic typing, so variables can hold different types of values.

Example:

    value = 10

    value = "Hello"

    value = true

### 4. Math operators

FluxScript supports basic and advanced arithmetic operators.

| Operator | Description | Example | Result |

|---|---|---|---|

| `+` | Addition | `5 + 3` | `8` |

| `-` | Subtraction | `5 - 3` | `2` |

| `*` | Multiplication | `5 * 3` | `15` |

| `/` | Division | `5 / 2` | `2.5` |

| `//` | Floor division | `7 // 2` | `3` |

| `%` | Modulo | `10 % 3` | `1` |

| `^` | Exponentiation | `2 ^ 3` | `8` |

Example:

    out 5 + 5

    out 10 - 3

    out 4 * 5

    out 10 / 2

    out 7 // 2

    out 10 % 3

    out 2 ^ 4

#### Order of operations

FluxScript follows mathematical operator precedence.

Example:

    out 5 + 5 * 2

Output:

    15

You can use parentheses to change the order.

Example:

    out (5 + 5) * 2

Output:

    20

### 5. Assignment operators

FluxScript supports compound assignment operators.

| Operator | Meaning |

|---|---|

| `+=` | Add and assign |

| `-=` | Subtract and assign |

| `*=` | Multiply and assign |

| `/=` | Divide and assign |

| `%=` | Modulo and assign |

| `^=` | Exponentiate and assign |

Example:

    x = 10

    x += 5

    out x

    x -= 3

    out x

    x *= 2

    out x

    x /= 4

    out x

    x %= 4

    out x

    x ^= 3

    out x

Output:

    15

    12

    24

    6

    2

    8

### 6. User input

FluxScript supports user input using `input()`.

You can use it to ask the user for information.

Example:

    name = input("What is your name? ")

    out name

You can also use `input()` without a prompt.

Example:

    answer = input()

    out answer

Starting with v0.5.0, `input()` automatically detects numbers, booleans, and strings.

Example:

    age = input("Enter your age: ")

    out type(age)

If the user enters `18`, the result is a number.

If the user enters `true`, the result is a boolean.

Other text is returned as a string.

### 7. IF / ELIF / ELSE

`if` checks whether a condition is true.

`elif` checks additional conditions.

`else` runs when none of the previous conditions are true.

Parentheses are not required around conditions.

Syntax:

    if condition {

        out "First condition"

    } elif anotherCondition {

        out "Second condition"

    } else {

        out "No conditions matched"

    }

Example:

    score = 85

    if score >= 90 {

        out "Excellent"

    } elif score >= 70 {

        out "Good"

    } else {

        out "Keep practicing"

    }

Output:

    Good

#### Comparison operators

| Operator | Description |

|---|---|

| `==` | Equal |

| `!=` | Not equal |

| `<` | Less than |

| `>` | Greater than |

| `<=` | Less than or equal |

| `>=` | Greater than or equal |

#### Logical operators

FluxScript supports `and`, `or`, and `not`.

Example:

    age = 20

    if age >= 18 and age < 60 {

        out "Adult"

    }

    if age < 18 or age >= 60 {

        out "Other age group"

    }

    enabled = false

    if not enabled {

        out "Disabled"

    }

#### The IN operator

The `in` operator checks whether a value exists inside an array or whether a string contains another string.

Example:

    fruits = ["apple", "banana", "orange"]

    out "banana" in fruits

    out "grape" in fruits

    out "World" in "Hello World"

Output:

    true

    false

    true

### 8. Ternary operator

FluxScript v0.6.0 introduces ternary conditional expressions.

A ternary expression allows you to choose between two values using a condition.

Syntax:

    result = valueIfTrue if condition else valueIfFalse

Example:

    age = 18

    status = "Adult" if age >= 18 else "Minor"

    out status

Output:

    Adult

You can also use ternary expressions inside functions.

Example:

    func checkAge(age) {

        return "Adult" if age >= 18 else "Minor"

    }

    out checkAge(15)

Output:

    Minor

### 9. WHILE loops

`while` repeats a block of code while a condition remains true.

Parentheses around the condition are optional.

Syntax:

    while condition {

        // Code

    }

Example:

    i = 0

    while i < 5 {

        out i

        i += 1

    }

Output:

    0

    1

    2

    3

    4

### 10. FOR loops

FluxScript supports several types of `for` loops.

#### Basic FOR loops

Syntax:

    for i(n) {

        out i

    }

Here, `i` is the loop variable and `n` is the number of iterations.

The loop starts at 0 and stops before `n`.

Example:

    for i(5) {

        out i

    }

Output:

    0

    1

    2

    3

    4

#### FOR loops with start and end

You can specify a starting value and an ending value.

Example:

    for i(2, 6) {

        out i

    }

Output:

    2

    3

    4

    5

#### FOR loops with step

You can also specify the step value.

Example:

    for i(0, 10, 2) {

        out i

    }

Output:

    0

    2

    4

    6

    8

#### FOR IN loops

You can iterate directly over an array.

Example:

    fruits = ["apple", "banana", "orange"]

    for fruit in fruits {

        out fruit

    }

Output:

    apple

    banana

    orange

### 11. BREAK / CONTINUE / PASS

FluxScript supports loop control statements.

#### BREAK

`break` immediately exits the current loop.

Example:

    for i(10) {

        if i == 5 {

            break

        }

        out i

    }

Output:

    0

    1

    2

    3

    4

#### CONTINUE

`continue` skips the remaining statements in the current iteration.

Example:

    for i(5) {

        if i == 2 {

            continue

        }

        out i

    }

Output:

    0

    1

    3

    4

#### PASS

`pass` is a statement that does nothing.

It can be used as a placeholder.

Example:

    if true {

        pass

    }

### 12. Arrays

Arrays can contain multiple values of different types.

Example:

    numbers = [10, 20, 30, 40]

    fruits = ["apple", "banana", "orange"]

    mixed = [1, "hello", true]

#### Accessing array elements

Array indexing starts at 0.

Example:

    numbers = [10, 20, 30]

    out numbers[0]

    out numbers[1]

    out numbers[2]

Output:

    10

    20

    30

#### Modifying array elements

You can modify elements using indexed assignment.

Example:

    numbers = [10, 20, 30]

    numbers[1] = 50

    out numbers

Output:

    [10, 50, 30]

#### Array methods

FluxScript supports methods for modifying arrays.

Methods do not use parentheses.

Example:

    numbers = [1, 2, 3]

    numbers.push 4

    numbers.append 5

    out numbers

Output:

    [1, 2, 3, 4, 5]

You can remove an element using `.remove`.

Example:

    numbers = [10, 20, 30]

    numbers.remove 20

    out numbers

Output:

    [10, 30]

The `.pop` method removes the last element.

Example:

    numbers = [10, 20, 30]

    numbers.pop

    out numbers

Output:

    [10, 20]

#### Sorting arrays

FluxScript v0.6.0 introduces `.sort` and `.rev`.

`.sort` sorts an array in ascending order.

`.rev` reverses the current order of an array.

Example:

    numbers = [5, 2, 8, 1, 9]

    numbers.sort

    out numbers

Output:

    [1, 2, 5, 8, 9]

To sort in descending order, use `.sort` followed by `.rev`.

Example:

    numbers.rev

    out numbers

Output:

    [9, 8, 5, 2, 1]

**Important:** `.rev` does not sort an array. It only reverses its current order.

#### Finding and counting elements

The `.index` method returns the index of an element.

The `.count` method counts how many times an element appears.

Example:

    numbers = [5, 2, 8, 2, 9]

    out numbers.index 8

    out numbers.count 2

Output:

    2

    2

### 13. Dictionaries

FluxScript v0.6.0 introduces dictionaries.

Dictionaries store values using keys instead of numeric indexes.

Currently, dictionary keys must be strings.

Example:

    player = {

        "name": "Alex",

        "level": 10,

        "health": 100

    }

#### Accessing dictionary values

Use a key inside square brackets.

Example:

    out player["name"]

    out player["level"]

Output:

    Alex

    10

#### Modifying dictionary values

You can modify existing values.

Example:

    player["level"] = 11

    out player["level"]

Output:

    11

You can also add new keys.

Example:

    player["gold"] = 500

    out player["gold"]

Output:

    500

#### Removing dictionary keys

Use `.remove` to remove a key.

Example:

    player.remove "health"

#### Dictionary keys and values

Use `.keys` to get the dictionary's keys.

Use `.values` to get its values.

Example:

    out player.keys

    out player.values

Dictionaries can store numbers, strings, booleans, arrays, and other dictionaries.

### 14. String methods

FluxScript supports several string methods.

Methods use parentheses-free syntax.

#### Uppercase and lowercase

Example:

    text = "Hello World"

    out text.upper

    out text.lower

Output:

    HELLO WORLD

    hello world

#### Replacing text

Example:

    text = "Hello World"

    out text.replace "World", "FluxScript"

Output:

    Hello FluxScript

#### Splitting strings

Example:

    text = "apple,banana,orange"

    out text.split ","

Output:

    ["apple", "banana", "orange"]

#### Appending text

Example:

    text = "Hello"

    text.append " World"

    out text

Output:

    Hello World

#### Trimming whitespace

FluxScript v0.6.0 introduces `.trim`.

Example:

    text = "  Hello World  "

    out text.trim

Output:

    Hello World

#### Checking prefixes and suffixes

Use `.startswith` and `.endswith`.

Example:

    text = "Hello World"

    out text.startswith "Hello"

    out text.endswith "World"

Output:

    true

    true

#### Finding text

Use `.find` to find the position of a substring.

Example:

    text = "Hello World"

    out text.find "World"

Output:

    6

#### Checking whether text contains another string

Use `.has`.

Example:

    text = "Hello World"

    out text.has "Hello"

    out text.has "Python"

Output:

    true

    false

### 15. String indexing

FluxScript v0.6.0 supports string indexing.

You can access individual characters using square brackets.

Indexing starts at 0.

Example:

    text = "Hello"

    out text[0]

    out text[1]

    out text[4]

Output:

    H

    e

    o

**Note:** String indexing currently operates on bytes, so multibyte UTF-8 characters may not behave as expected.

### 16. Functions

Functions allow you to reuse code.

Use `func` to create a function.

Syntax:

    func functionName(parameter) {

        return parameter

    }

Example:

    func add(a, b) {

        return a + b

    }

    out add(5, 3)

Output:

    8

#### Functions with multiple operations

Example:

    func square(x) {

        return x ^ 2

    }

    func cube(x) {

        return x ^ 3

    }

    out square(5)

    out cube(3)

Output:

    25

    27

Functions can also execute statements without explicitly returning a value.

Example:

    func greet(name) {

        out "Hello, " + name

    }

    greet("FluxScript")

Output:

    Hello, FluxScript

#### Default parameters

FluxScript v0.6.0 introduces default function parameters.

A default parameter is used when the caller does not provide an argument.

Example:

    func greet(name = "World") {

        out "Hello, " + name

    }

    greet()

    greet("Alex")

Output:

    Hello, World

    Hello, Alex

Default parameters must come after required parameters.

Example:

    func greet(message, name = "World") {

        out message + ", " + name

    }

    greet("Hello")

    greet("Welcome", "Alex")

Output:

    Hello, World

    Welcome, Alex

### 17. Built-in functions

FluxScript provides several built-in functions.

| Function | Description |

|---|---|

| `int()` | Convert a value to an integer |

| `float()` | Convert a value to a number |

| `str()` | Convert a value to a string |

| `bool()` | Convert a value to a boolean |

| `type()` | Return the value's type |

| `len()` | Return the length of a string, array, or dictionary |

| `input()` | Read user input |

| `timestamp()` | Return the current Unix timestamp |

| `wait()` | Pause execution |

Example:

    out int("10")

    out float("3.14")

    out str(123)

    out bool(1)

    out type("Hello")

    out len([10, 20, 30])

Output:

    10

    3.14

    123

    true

    string

    3

#### Waiting

Use `wait(seconds)` to pause execution.

Example:

    out "Starting..."

    wait(2)

    out "Finished!"

#### Unix timestamps

Use `timestamp()` to get the current Unix timestamp.

Example:

    out timestamp()

The timestamp is returned in seconds.

### 18. File operations

FluxScript supports reading and writing files.

#### Reading files

Use `readfile()` to read a file's contents.

Example:

    text = readfile("notes.txt")

    out text

#### Writing files

Use `writefile()` to create or overwrite a file.

Example:

    writefile("notes.txt", "Hello FluxScript!")

#### Appending files

Use `addfile()` to append text to an existing file.

Example:

    addfile("notes.txt", "\nAnother line!")

#### Checking whether a file exists

Use `fileexists()`.

Example:

    if fileexists("notes.txt") {

        out "File exists!"

    } else {

        out "File not found!"

    }

### 19. Error handling

FluxScript supports `try` and `catch` for error handling.

The `try` block contains code that may fail.

The `catch` block handles the error.

Example:

    try {

        out 10 / 0

    } catch {

        out "An error occurred!"

    }

Output:

    An error occurred!

#### Accessing error messages

FluxScript v0.6.0 allows you to access the captured error message using `catch(error)`.

Example:

    try {

        out 10 / 0

    } catch(error) {

        out error

    }

Output:

    ZeroDivisionError: division by zero

This is useful for debugging and displaying error information.

### 20. Modules

FluxScript supports a module system.

Modules allow you to organize functions into separate files.

Use `get` to load a module.

#### Module directory

Modules are stored in the `module` folder.

Example:

    FluxScript/

    ├── flux.exe

    ├── main.flux

    └── module/

        ├── math.flux

        ├── time.flux

        ├── random.flux

        └── string.flux

#### Importing a module

Example:

    get math

    out math.sq(5)

Modules are written in FluxScript itself.

You can call module functions using:

    moduleName.functionName(arguments)

#### Math module

The `math.flux` module provides mathematical utility functions.

Example:

    get math

    out math.sq(5)

    out math.cube(3)

    out math.factorial(5)

Output:

    25

    27

    120

#### Time module

The `time.flux` module provides Unix timestamp and UTC time utilities.

Example:

    get time

    out time.now()

    out time.hours()

    out time.minutes()

#### Random module

The `random.flux` module provides pseudo-random number generation.

Example:

    get random

    out random.randint(1, 10)

    out random.rand()

    fruits = ["apple", "banana", "orange"]

    out random.choice(fruits)

Random results may be different each time.

#### String module

FluxScript v0.6.0 introduces `string.flux`.

It provides additional functions for working with strings.

Example:

    get string

    out string.repeat("Hi", 3)

    out string.reverse("Hello")

Output:

    HiHiHi

    olleH

**Important:** Module functions use parentheses, while methods do not.

The module directory is resolved relative to the interpreter's working directory, so run `flux.exe` from the FluxScript project folder.

### 21. Native console operations (v0.8.0)

FluxScript v0.8.0 added native console functions for Windows terminal control.

You can use these functions directly, or create aliases in a FluxScript module such as `module/console.flux`.

#### Console functions

| Function | Description |

|---|---|

| `consoleClear()` | Clear the console screen |

| `consoleTitle(text)` | Set the console window title |

| `consoleColor(name)` | Change foreground color |

| `consoleBackground(name)` | Change background color |

| `consoleReset()` | Restore default console colors |

| `consoleCursor(x, y)` | Move the cursor |

| `consolePosition()` | Return the cursor position |

| `consoleSize()` | Return the console dimensions |

| `consoleResize(width, height)` | Resize the console |

| `consoleHideCursor()` | Hide the cursor |

| `consoleShowCursor()` | Show the cursor |

| `consoleReadKey()` | Read a key |

| `consoleKeyAvailable()` | Check whether a key is waiting |

| `consolePause()` | Wait for a key press |

| `consoleBeep(frequency, duration)` | Play a beep |

| `consoleProgress(percent, width)` | Print a progress bar |

| `consoleBox(text)` | Print a formatted text box |

| `consoleMenu(options)` | Show a selection menu |

The `console.flux` module can expose shorter names such as `console.clear()`, `console.title(text)`, `console.color(name)`, and `console.progress(percent, width)`.

Example with native functions:

    consoleTitle("FluxScript Console")

    consoleColor("cyan")

    out "Hello from FluxScript v1.0.0!"

    consoleProgress(75, 30)

    consoleReset()

Console color names include `black`, `blue`, `green`, `cyan`, `red`, `magenta`, `yellow`, and `white`.

### 22. Native graphical interfaces (v0.9.0)

FluxScript v0.9.0 introduces graphical user interfaces using the Windows Win32 API.

Applications can create native windows, labels, buttons, and textboxes without an external GUI framework.

#### GUI functions

| Function | Description |

|---|---|

| `windowAdd(title, size)` | Create a native window |

| `windowBackground(color)` | Set the active window background |

| `windowResizable(enabled)` | Enable or disable resizing |

| `windowLabel(size, color, position, text)` | Add a text label |

| `windowButton(size, color, position, text)` | Add a clickable button |

| `windowTextbox(size, color, position, text)` | Add a text input field |

#### Dimension and position literals

Use `widthxheight` notation for dimensions and positions.

Examples:

    500x350

    180x45

    20x90

The `x` is part of a dimension literal, not a multiplication operator.

#### Creating a window

Example:

    func -ready() {

        main = windowAdd("FluxScript 1.0.0", 500x350) {

            windowBackground("black")

            windowResizable(false)

            windowLabel(350x40, "white", 20x20, "FLUXSCRIPT 1.0.0")

            button = windowButton(180x45, "blue", 20x90, "CLICK ME")

            button.onclick {

                out "Button clicked!"

            }

            windowTextbox(250x35, "white", 20x160, "Type here...")

        }

    }

Save this example as `test_gui.flux` and run it using `flux.exe test_gui.flux`.

The window opens with a black background, a white label, a blue button, and a textbox.

Clicking the button prints `Button clicked!` in the console.

#### Window context blocks

Use `.context` to select the window that receives new controls and settings.

Example:

    func -ready() {

        main = windowAdd("My Window", 400x300)

        main.context {

            windowBackground("blue")

            windowResizable(false)

            windowLabel(200x30, "white", 20x20, "Hello!")

        }

    }

A context block makes the selected window the active parent for its GUI operations.

### 23. GUI events and control properties (v1.0.0)

FluxScript v1.0.0 expands native Win32 GUI controls and interaction.

#### Control properties

| Property | Description |

|---|---|

| `.text` | Read or modify control text |

| `.color` | Change control color |

| `.position` | Read or modify position |

| `.size` | Read or modify dimensions |

| `.visible` | Show or hide a control |

| `.enabled` | Enable or disable a control |

| `.font` | Configure the font |

| `.bold` | Enable or disable bold text |

The default GUI font is Arial.

#### Events

| Event | Description |

|---|---|

| `.onclick` | Left mouse click |

| `.onrclick` | Right mouse click |

| `.onchange` | Control value changes |

| `.detectmouse` | Mouse enters or leaves a control |

| `.detectMouseMoving` | Mouse movement |

`.detectmouse` changes to `true` once when the pointer enters and to `false` once when it leaves, without repeatedly firing while the pointer remains inside.

Keyboard shortcuts and key combinations are also supported.

#### Additional controls and images

GUI controls include windows, labels, buttons, textboxes, checkboxes, sliders, and progress bars.

Images support PNG, JPG, and JPEG formats. Image lookup can also fall back to the `flux/` directory.

#### Dialogs and timers

`alert()` displays a message dialog.

`confirm()` displays a confirmation dialog and returns a boolean.

Non-blocking, single-execution timers allow an action to run later without blocking the interface.

#### Platform notes

The native GUI targets Windows and uses Win32.

GUI programs run through the FluxScript interpreter.

The optional `gui.flux` wrapper module is separate from the native GUI implementation.

### 24. Combining everything

Here is an example using several FluxScript features together.

    get random

    func attack(player, enemy, damage = 10) {

        enemy["health"] -= damage

        out player["name"] + " dealt " + str(damage) + " damage!"

    }

    player = {

        "name": "Alex",

        "health": 100,

        "level": 5

    }

    enemy = {

        "name": "Goblin",

        "health": 30

    }

    out "FluxScript v1.0.0"

    while enemy["health"] > 0 {

        damage = random.randint(5, 15)

        attack(player, enemy, damage)

    }

    out "Enemy defeated!"

This example demonstrates dictionaries, functions, default parameters, modules, random numbers, and loops.

### 25. Mini project: Number Guessing Game

Here is a simple game written entirely in FluxScript.

    get random

    func game() {

        secret = random.randint(1, 20)

        attempts = 0

        out "NUMBER GUESSING GAME"

        out "Guess a number between 1 and 20!"

        while true {

            guess = input("Your guess: ")

            attempts += 1

            if guess == secret {

                out "Correct!"

                out "Attempts: " + str(attempts)

                break

            } elif guess < secret {

                out "Too low!"

            } else {

                out "Too high!"

            }

        }

    }

    game()

The game uses variables, functions, loops, conditions, random numbers, and automatic input type detection.

### 26. Aliases

FluxScript v1.0.0 introduces aliases.

An alias can use any valid identifier and may refer to another alias.

Aliases are local to the files where they are declared.

The exact alias declaration syntax is not specified in this tutorial.

### 27. Classes and inheritance

FluxScript v1.0.0 introduces class inheritance.

A derived class can inherit methods from a parent class.

Class declaration and inheritance syntax should be documented alongside the interpreter implementation.

### 28. Lambda expressions

Lambda expressions provide anonymous functions.

Syntax:

    func(x): expression

Example expression:

    func(x): x * 2

### 29. Functional array operations

FluxScript v1.0.0 adds `map()`, `filter()`, and `reduce()`.

| Function | Description |

|---|---|

| `map()` | Transform array elements |

| `filter()` | Select elements matching a condition |

| `reduce()` | Combine elements into a result |

These operations can be used with functions and lambda expressions.

### 30. Named arguments

Named arguments allow function calls to specify parameter names.

Syntax:

    functionName(name=value)

Named arguments improve readability when functions have multiple parameters.

### 31. Complex numbers

FluxScript v1.0.0 adds support for numbers with real and imaginary components.

Exact literal syntax and supported complex-number operations depend on the interpreter implementation.

### 32. Assertions

Use `assert` to check a condition and raise an error when it is false.

Example:

    age = 18

    assert age >= 0, "Age cannot be negative"

Assertions can help validate input and program state.

### 33. Hierarchical packages

FluxScript v1.0.0 adds hierarchical packages to organize modules into subdirectories.

Modules continue to be imported with `get`.

The exact import notation for nested packages should match the interpreter's package resolver.

### 34. New string features

#### Newline escapes

Strings support `\n` to represent a newline.

Example:

    out "First line\nSecond line"

#### Raw strings

Raw strings preserve backslashes and other escape sequences literally.

The exact raw-string prefix or delimiter is not specified in this tutorial.

### 35. Regular expressions

The `regex.flux` module provides regular expression operations using a recursive backtracking engine written in FluxScript.

Example:

    get regex

    out regex.search("(cat|dog)+", "xxcatdogyy")

    out regex.findall("(ab|cd)+", "abab xx cdcd")

    out regex.fullmatch("[A-Z][a-z]{2,5}", "Hello")

    out regex.group("(\\d+)-(\\w+)", "123-Alex", 1)

    out regex.groups("(\\d+)-(\\w+)", "123-Alex")

    out regex.search("a+?", "aaaa")

    out regex.search("(ab|a)b", "ab")

The module supports character classes, alternation, capturing groups, and greedy and lazy quantifiers.

Advanced edge cases and performance improvements are planned for v1.1.0.

### 36. Serialization

The `serialize.flux` module converts supported values to strings and reconstructs them.

Functions:

| Function | Description |

|---|---|

| `serialize.dumps(value)` | Serialize a value |

| `serialize.loads(data)` | Deserialize a string |

| `serialize.clone(value)` | Create a deep copy of supported data |

| `serialize.isvalid(data)` | Check whether data can be deserialized |

Example:

    get serialize

    player = {"name": "Alex", "level": 10}

    data = serialize.dumps(player)

    restored = serialize.loads(data)

    out restored["name"]

This initial module uses `json.flux` and supports JSON-compatible values. Functions and cyclic references are not supported.

### 37. Persistent key-value storage

The `storage.flux` module provides key-value persistence, conceptually similar to Python's `shelve`.

Functions:

| Function | Description |

|---|---|

| `storage.open(path)` | Open or initialize a database |

| `storage.set(db, key, value)` | Store a value |

| `storage.get(db, key)` | Retrieve a value |

| `storage.has(db, key)` | Check for a key |

| `storage.remove(db, key)` | Remove a key |

| `storage.keys(db)` | Retrieve keys |

| `storage.values(db)` | Retrieve values |

| `storage.clear(db)` | Remove all entries |

| `storage.save(db)` | Save the database |

| `storage.close(db)` | Save and close the database |

Example:

    get storage

    db = storage.open("players.db")

    storage.set(db, "player1", {"name": "Alex", "level": 10})

    out storage.get(db, "player1")

    storage.close(db)

The module uses `serialize.flux` and native file operations for persistence.

The serialization and storage examples describe the proposed module APIs and require testing against the v1.0.0 interpreter.

## IMPORTANT NOTES

- FluxScript is still in development.

- FluxScript v1.0.0 extends native Win32 GUI controls, events, and properties.

- GUI window and control sizes use `WxH` literals.

- GUI events include `.onclick`, `.onrclick`, `.onchange`, `.detectmouse`, and `.detectMouseMoving`.

- GUI applications can start from `func -ready() { }`.

- FluxScript v0.8.0 added native console operations.

- FluxScript uses C++17 and MSVC for its Windows interpreter.

- Use `{ }` for blocks.

- Statements do not require semicolons.

- `if`, `elif`, and `while` do not require parentheses around conditions.

- `for i(n)` starts at 0 and runs `n` times.

- Extended `for` loops support start, end, and step values.

- `break`, `continue`, and `pass` are supported.

- Arrays and strings use zero-based indexing.

- Dictionaries currently require string keys.

- `.sort` sorts arrays in ascending order.

- `.rev` only reverses the current array order.

- Methods use parentheses-free syntax.

- Functions and module functions use parentheses for arguments.

- Default function parameters are supported.

- `input()` automatically detects numbers, booleans, and strings.

- `catch(error)` provides access to error messages.

- `get` loads modules from the `module` directory.

- FluxScript modules are written in FluxScript itself.

- Both `.flux` and `.flx` files can be used with the interpreter.

- v1.0.0 adds aliases, inheritance, lambdas, named arguments, complex numbers, assertions, hierarchical packages, regex, serialization, and persistent storage.

- Some features may change in future versions.

That's it! Thanks for trying FluxScript v1.0.0!

