get random
get string

--[[
Treasure Hunter
FluxScript v0.6.0 Minigame
]]

func line() {
    out "=================================="
}

func showStatus(player) {
    line()
    out "PLAYER STATUS"
    out "Name: " + player["name"]
    out "HP: " + str(player["hp"])
    out "Gold: " + str(player["gold"])
    out "Potions: " + str(player["potions"])
    out "Level: " + str(player["level"])
    line()
}

func heal(player, amount = 25) {
    player["hp"] += amount

    if player["hp"] > 100 {
        player["hp"] = 100
    }

    out "Recovered " + str(amount) + " HP!"
}

func battle(player) {
    monsters = ["Slime", "Goblin", "Skeleton", "Orc"]
    monster = random.choice(monsters)

    monsterHP = random.randint(15, 35)

    out ""
    out "A wild " + monster + " appeared!"

    while monsterHP > 0 and player["hp"] > 0 {
        out ""
        out "Your HP: " + str(player["hp"])
        out monster + " HP: " + str(monsterHP)

        out "1. Attack"
        out "2. Use potion"
        out "3. Run"

        action = input("Choose: ")

        if action == 1 {
            damage = random.randint(8, 20)
            monsterHP -= damage

            out "You dealt " + str(damage) + " damage!"

            if monsterHP <= 0 {
                reward = random.randint(10, 30)
                player["gold"] += reward
                player["level"] += 1

                out "Monster defeated!"
                out "You earned " + str(reward) + " gold!"
                break
            }

            enemyDamage = random.randint(4, 14)
            player["hp"] -= enemyDamage

            out "The monster dealt " + str(enemyDamage) + " damage!"

        } elif action == 2 {
            if player["potions"] > 0 {
                player["potions"] -= 1
                heal(player)
            } else {
                out "No potions left!"
            }

        } elif action == 3 {
            out "You escaped!"
            break

        } else {
            out "Invalid action!"
        }
    }
}

func treasure(player) {
    gold = random.randint(10, 50)
    player["gold"] += gold

    out ""
    out "You found a treasure chest!"
    out "You collected " + str(gold) + " gold!"
}

func shop(player) {
    out ""
    out "POTION SHOP"
    out "Potion price: 20 gold"

    choice = input("Buy potion? (yes/no): ")
    choice = choice.trim

    if choice == "yes" {
        if player["gold"] >= 20 {
            player["gold"] -= 20
            player["potions"] += 1
            out "Potion purchased!"
        } else {
            out "Not enough gold!"
        }
    }
}

func showInventory(player) {
    out ""
    out "INVENTORY"

    items = ["Sword", "Shield", "Potion", "Map"]

    items.sort
    out "Sorted items:"
    out items

    items.rev
    out "Reversed items:"
    out items

    out "Has Sword: " + str("Sword" in items)
}

func main() {
    line()
    out "TREASURE HUNTER"
    out "FluxScript v0.6.0"
    line()

    name = input("Enter your name: ")
    name = name.trim

    name = "Adventurer" if name == "" else name

    player = {
        "name": name,
        "hp": 100,
        "gold": 0,
        "potions": 2,
        "level": 1
    }

    out ""
    out "Welcome, " + player["name"] + "!"

    playing = true

    while playing and player["hp"] > 0 {
        out ""
        line()
        out "1. Explore"
        out "2. Shop"
        out "3. Inventory"
        out "4. Status"
        out "5. Quit"
        line()

        choice = input("Choose: ")

        if choice == 1 {
            event = random.randint(1, 3)

            if event == 1 {
                battle(player)
            } elif event == 2 {
                treasure(player)
            } else {
                out "Nothing happened..."
            }

        } elif choice == 2 {
            shop(player)

        } elif choice == 3 {
            showInventory(player)

        } elif choice == 4 {
            showStatus(player)

        } elif choice == 5 {
            playing = false

        } else {
            out "Invalid option!"
        }
    }

    line()

    if player["hp"] <= 0 {
        out "GAME OVER!"
    } else {
        out "Thanks for playing!"
    }

    out "Final level: " + str(player["level"])
    out "Final gold: " + str(player["gold"])
}

try {
    main()
} catch(error) {
    out "Game error: " + error
}