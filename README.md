
# FluxScript-interpreter

FluxScript is an indie programming language created on October 24, 2026. It is designed to be simple, fast, and easy to learn.

I hope that in future versions, FluxScript will be able to create games and applications and become popular. Not for fame, but so people can code something with less struggle.

**Current version: 0.4.0**

FluxScript currently supports variables, printing, user input, math, conditionals, loops, arrays, functions, and modules.

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

You can also use the `.flx` extension:

    flux.exe test.flx

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

Open a `.flux` or `.flx` file.

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

## FLUXSCRIPT v0.4.0 TUTORIAL

### 1. Printing text and values

FluxScript uses `out` to print text and values, similar to Python's `print()`.

You don't need parentheses or semicolons.

Example:

    out "Hello, World!"

    out 'Hello again!'

    out 123

    out 5 + 5

Both single and double quotation marks are supported for strings.

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

**Important:** Since `//` is also used for floor division in v0.4.0, it is recommended to put single-line comments on their own lines.

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

Variables can store numbers, strings, booleans, and arrays.

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

FluxScript v0.4.0 supports compound assignment operators.

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

Currently, `input()` returns a string.

### 7. IF / ELSE

`if` checks whether a condition is true.

If the condition is false, the optional `else` block can run.

In v0.4.0, parentheses are no longer required around conditions.

Syntax:

    if condition {

        out "Condition is true"

    } else {

        out "Condition is false"

    }

Example:

    x = 10

    if x > 5 {

        out "Big"

    } else {

        out "Small"

    }

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

FluxScript supports `and` and `or`.

Example:

    age = 20

    if age >= 18 and age < 60 {

        out "Adult"

    }

    if age < 18 or age >= 60 {

        out "Other age group"

    }

### 8. WHILE loops

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

### 9. FOR loops

FluxScript v0.4.0 introduces `for` loops.

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

You can also use variables to determine the number of iterations.

Example:

    count = 3

    for i(count) {

        out "Hello!"

    }

This prints `Hello!` three times.

### 10. Arrays

FluxScript v0.4.0 supports arrays.

Arrays can contain multiple values.

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

#### Arrays with loops

Example:

    numbers = [10, 20, 30]

    for i(3) {

        out numbers[i]

    }

Output:

    10

    20

    30

**Note:** Direct array element assignment, such as `numbers[0] = 5`, is not yet supported.

### 11. Functions

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

### 12. Modules

FluxScript v0.4.0 introduces a module system.

Modules allow you to organize functions into separate files.

Use `get` to load a module.

#### Module directory

Modules are stored in the `module` folder.

Example:

    flux/

    ├── flux.exe

    ├── main.flux

    └── module/

        └── math.flux

#### Creating a module

Create `module/math.flux`:

    func square(x) {

        return x ^ 2

    }

    func cube(x) {

        return x ^ 3

    }

    func add(a, b) {

        return a + b

    }

#### Importing a module

In `main.flux`, type:

    get math

    out math.square(5)

    out math.cube(3)

    out math.add(10, 20)

Output:

    25

    27

    30

When you use `get math`, FluxScript searches for `module/math.flux`.

You can call functions from the module using:

    moduleName.functionName(arguments)

The module directory is resolved relative to the interpreter's working directory, so run `flux.exe` from the FluxScript project folder.

### 13. Combining everything

Here is an example using several v0.4.0 features together.

    numbers = [2, 4, 6, 8, 10]

    func square(x) {

        return x ^ 2

    }

    out "FluxScript v0.4.0"

    for i(5) {

        number = numbers[i]

        if number % 2 == 0 {

            out square(number)

        }

    }

Output:

    FluxScript v0.4.0

    4

    16

    36

    64

    100

## IMPORTANT NOTES

- FluxScript is still in development.

- Use `{ }` for blocks.

- `if` and `while` no longer require parentheses around conditions.

- `for i(n)` starts at 0 and runs `n` times.

- Arrays use zero-based indexing.

- `get` loads modules from the `module` directory.

- Both `.flux` and `.flx` files can be used with the interpreter.

- Some features may change in future versions.

That's it! Thanks for trying FluxScript!
