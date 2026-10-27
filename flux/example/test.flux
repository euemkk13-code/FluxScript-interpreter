
out "========== FLUXSCRIPT 0.9.0 =========="

func -ready() {
    main = windowAdd("FluxScript 0.9.0 - GUI Test", 500x350) {
        windowBackground("black")
        windowResizable(false)

        windowLabel(350x40, "white", 20x20, "FLUXSCRIPT 0.9.0")

        button = windowButton(180x45, "blue", 20x90, "CLICK HERE")

        button.onclick {
            out "YEEEEEEEEEESSSSSSS WORKED"
        }

        windowTextbox(250x35, "white", 20x160, "Type something...")
    }
}
