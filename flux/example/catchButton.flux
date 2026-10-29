--[[
Catch the Button!
FluxScript 1.0.0
Timer Edition
]]

get random

score = 0
goal = 10
gameRunning = true

func moveTarget() {
    x = random.randint(30, 430)
    y = random.randint(155, 290)

    target.position = [x, y]
}

windowAdd("Catch the Button! - FluxScript", 600x450) {
    windowBackground("#181818")
    windowResizable = false

    title = windowLabel(500x40, "cyan", 30x20, "CATCH THE BUTTON!")
    title.font = "Arial"
    title.bold = true

    scoreLabel = windowLabel(400x35, "white", 30x70, "Score: 0 / 10")
    scoreLabel.bold = true

    instruction = windowLabel(500x30, "gray", 30x110, "Catch it before it moves!")

    target = windowButton(130x45, "blue", 220x190, "CATCH ME!")

    progress = windowProgressBar(500x25, 30x360, "green", "gray", 0, 10)

    func scheduleMove() {
        timer(1) {
            if gameRunning {
                moveTarget()
                scheduleMove()
            }
        }
    }

    target.onclick {
        if score < goal {
            score += 1

            scoreLabel.text = "Score: " + str(score) + " / 10"
            progress.value = score

            moveTarget()

            if score == 5 {
                instruction.text = "HALFWAY THERE!"
                instruction.color = "yellow"
            }

            if score == goal {
                gameRunning = false

                title.text = "YOU WIN!"
                title.color = "green"

                instruction.text = "Congratulations!"
                instruction.color = "green"

                target.text = "PLAY AGAIN"
                target.color = "green"
                target.position = [220, 190]
            }
        } else {
            score = 0
            gameRunning = true

            title.text = "CATCH THE BUTTON!"
            title.color = "cyan"

            scoreLabel.text = "Score: 0 / 10"
            instruction.text = "Catch it before it moves!"
            instruction.color = "gray"

            progress.value = 0

            target.text = "CATCH ME!"
            target.color = "blue"

            moveTarget()
            scheduleMove()
        }
    }

    scheduleMove()
}
