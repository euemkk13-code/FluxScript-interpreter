FluxScript v0.9.0 GUI — Windows 7+ / Win32, C++17

Copy main.cpp to project root and the src/ files to src/.
The module/gui.flux is NOT included, per request.

MSVC developer command prompt:
cl /nologo /EHsc /std:c++17 /Isrc main.cpp src\lexer.cpp src\parser.cpp src\interpreter.cpp src\gui.cpp user32.lib gdi32.lib /Fe:flux.exe

Native functions:
windowAdd("Title", 400x300) -> numeric handle
windowBackground("black")
windowResizable(false)
windowLabel(200x30, "white", 10x10, "Text") -> handle
windowButton(120x30, "blue", 10x50, "Click") -> handle
windowTextbox(200x30, "white", 10x90, "Placeholder") -> handle

Examples:
func -ready() {
    main = windowAdd("FluxScript", 400x300) {
        windowBackground("black")
        windowResizable(false)
        btn = windowButton(120x30, "blue", 20x40, "Click")
        btn.onclick {
            out "Clicked!"
        }
    }
    main.context {
        windowBackground("gray")
    }
}

The GUI event loop runs after top-level execution / -ready.
The dimensions WxH are parsed as a 2-number array.
Window and control references are numeric IDs (initial implementation).

Limitations: native Win32 build was NOT tested on Windows/MSVC in this environment.
Textbox .context is parsed, but advanced textbox properties are deferred.
Button callback bodies are executed on the interpreter's event loop.
A windowAdd(...) without a block creates a default window.
The Win32 GUI is only compiled on Windows; there is no Linux/macOS backend.
