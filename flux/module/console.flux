--[[
FluxScript v0.8.0 - Console Module
Native console API wrappers.
Load with: get console
]]

func clear() {
    consoleClear()
}

func title(text) {
    consoleTitle(text)
}

func color(name) {
    consoleColor(name)
}

func background(name) {
    consoleBackground(name)
}

func reset() {
    consoleReset()
}

func cursor(x, y) {
    consoleCursor(x, y)
}

func position() {
    return consolePosition()
}

func size() {
    return consoleSize()
}

func resize(width, height) {
    consoleResize(width, height)
}

func hideCursor() {
    consoleHideCursor()
}

func showCursor() {
    consoleShowCursor()
}

func readKey() {
    return consoleReadKey()
}

func keyAvailable() {
    return consoleKeyAvailable()
}

func pause() {
    consolePause()
}

func beep(frequency, duration) {
    consoleBeep(frequency, duration)
}

func progress(percent, width) {
    consoleProgress(percent, width)
}

func box(text) {
    consoleBox(text)
}

func menu(options) {
    return consoleMenu(options)
}