get random
out "=== GUESS THE NUMBER ==="
out "Try to find the secret number from 1 to 10!"

name = input("Whats your name? ")
mensagem = "Good luck, "
mensagem.append name
out mensagem
wait(0.5)
out 'Loading game.'
wait(1)
out 'Loading game..'
wait(1)
out 'Loading game...'
wait(1)
secret = random.randint(1, 10)
tries = 0

while true {
    guess = input("Type your guess: ")
    tries += 1

    if guess == secret {
        out "CONGRATS! YOU WON!"
        out "Number of tries:"
        out tries
        break
    } elif guess < secret {
        out "Too low! Try an higher number."
    } else {
        out "Too high! Try a lower number."
    }
}

out "=== END OF GAME ==="
