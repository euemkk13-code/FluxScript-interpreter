
get random
get string
get json
get math
--[[
Treasure Hunter
FluxScript v0.7.0 RPG
]]
func line() {
    out "========================================"
}
func bar() {
    out "----------------------------------------"
}
func createPlayer(name) {
    return {
        "name": name,
        "hp": 100,
        "maxHp": 100,
        "gold": 25,
        "potions": 2,
        "level": 1,
        "xp": 0,
        "attack": 8,
        "defense": 2,
        "weapon": "Rusty Sword",
        "armor": "Cloth Armor",
        "inventory": ["Old Map", "Rusty Sword", "Cloth Armor"],
        "wins": 0,
        "steps": 0,
        "bossWins": 0
    }
}
func showStatus(player) {
    line()
    out "ADVENTURER STATUS"
    out f"Name: {player['name']}"
    out f"Level: {player['level']} | XP: {player['xp']}/{player['level'] * 35}"
    out f"HP: {player['hp']}/{player['maxHp']}"
    out f"Attack: {player['attack']} | Defense: {player['defense']}"
    out f"Gold: {player['gold']} | Potions: {player['potions']}"
    out f"Weapon: {player['weapon']}"
    out f"Armor: {player['armor']}"
    out f"Victories: {player['wins']} | Bosses: {player['bossWins']}"
    out f"Explorations: {player['steps']}"
    line()
}
func heal(player, amount = 35) {
    if player["potions"] <= 0 {
        out "You have no potions!"
        return false
    }
    if player["hp"] >= player["maxHp"] {
        out "Your HP is already full!"
        return false
    }
    before = player["hp"]
    player["potions"] -= 1
    player["hp"] += amount
    if player["hp"] > player["maxHp"] {
        player["hp"] = player["maxHp"]
    }
    recovered = player["hp"] - before
    out f"You recovered {recovered} HP!"
    return true
}
func gainXP(player, amount) {
    player["xp"] += amount
    out f"You earned {amount} XP!"
    while player["xp"] >= player["level"] * 35 {
        needed = player["level"] * 35
        player["xp"] -= needed
        player["level"] += 1
        player["maxHp"] += 12
        player["attack"] += 3
        player["defense"] += 1
        player["hp"] = player["maxHp"]
        line()
        out f"LEVEL UP! You are now level {player['level']}!"
        out "Your stats improved and HP was restored!"
        line()
    }
}
func rareDrop(player) {
    rareItems = ["Dragon Gem", "Ancient Relic", "Golden Crown", "Moon Crystal"]
    item = random.choice(rareItems)
    inventory = player["inventory"]
    inventory.push item
    out "***************************************"
    out f"RARE ITEM FOUND: {item}!"
    out "Added to your inventory!"
    out "***************************************"
}
func enemyAttack(player, monster, monsterAttack) {
    damage = random.randint(monsterAttack - 3, monsterAttack + 4) - player["defense"]
    if damage < 1 {
        damage = 1
    }
    if math.chance(10) {
        damage += 5
        out "The enemy landed a critical hit!"
    }
    player["hp"] -= damage
    if player["hp"] < 0 {
        player["hp"] = 0
    }
    out f"{monster} dealt {damage} damage!"
}
func battle(player, boss = false) {
    monsters = ["Slime", "Goblin", "Skeleton", "Orc", "Wolf", "Dark Knight"]
    monster = random.choice(monsters)
    monsterHP = random.randint(20, 35) + player["level"] * 7
    monsterAttack = random.randint(5, 10) + player["level"] * 2
    if boss {
        monster = "Ancient Dragon"
        monsterHP = 85 + player["level"] * 18
        monsterAttack = 12 + player["level"] * 3
        out "WARNING! A BOSS HAS APPEARED!"
    }
    out f"A wild {monster} appeared!"
    while monsterHP > 0 and player["hp"] > 0 {
        bar()
        out f"Your HP: {player['hp']}/{player['maxHp']}"
        out f"{monster} HP: {monsterHP}"
        out "1. Attack"
        out "2. Defend"
        out "3. Use potion"
        out "4. Run"
        action = input("Choose: ")
        defending = false
        turn = true
        if action == 1 {
            damage = random.randint(5, 12) + player["attack"]
            if math.chance(20) {
                damage *= 2
                out "CRITICAL HIT!"
            }
            monsterHP -= damage
            out f"You dealt {damage} damage!"
        } elif action == 2 {
            defending = true
            out "You brace for the next attack!"
        } elif action == 3 {
            turn = heal(player)
        } elif action == 4 {
            if boss {
                out "You cannot escape from the boss!"
            } elif math.chance(65) {
                out "You escaped successfully!"
                return
            } else {
                out "Escape failed!"
            }
        } else {
            out "Invalid action!"
            turn = false
        }
        if monsterHP <= 0 {
            reward = random.randint(12, 28) + player["level"] * 5
            xp = random.randint(12, 24) + player["level"] * 3
            if boss {
                reward *= 4
                xp *= 3
                player["bossWins"] += 1
            }
            player["gold"] += reward
            player["wins"] += 1
            out f"{monster} defeated! +{reward} gold!"
            gainXP(player, xp)
            if math.chance(25) {
                player["potions"] += 1
                out "The monster dropped a potion!"
            }
            if math.chance(5) {
                rareDrop(player)
            }
            return
        }
        if turn {
            if defending {
                reduced = monsterAttack - player["defense"]
                if reduced < 1 {
                    reduced = 1
                }
                damage = random.randint(1, reduced)
                player["hp"] -= damage
                if player["hp"] < 0 {
                    player["hp"] = 0
                }
                out f"Blocked most damage! Took {damage} HP."
            } else {
                enemyAttack(player, monster, monsterAttack)
            }
        }
    }
}
func treasure(player) {
    gold = random.randint(15, 55) + player["level"] * 3
    player["gold"] += gold
    out f"You found a treasure chest with {gold} gold!"
    if math.chance(33) {
        player["potions"] += 1
        out "Bonus: one healing potion!"
    }
    if math.chance(8) {
        rareDrop(player)
    }
}
func shrine(player) {
    out "You discovered an ancient healing shrine!"
    amount = random.randint(15, 40)
    player["hp"] += amount
    if player["hp"] > player["maxHp"] {
        player["hp"] = player["maxHp"]
    }
    out f"The shrine restored your HP to {player['hp']}!"
}
func trap(player) {
    damage = random.randint(5, 17) + player["level"]
    damage -= player["defense"]
    if damage < 1 {
        damage = 1
    }
    player["hp"] -= damage
    if player["hp"] < 0 {
        player["hp"] = 0
    }
    out f"A hidden trap dealt {damage} damage!"
}
func explore(player) {
    player["steps"] += 1
    out f"Exploration #{player['steps']}"
    event = random.randint(1, 100)
    if player["steps"] % 7 == 0 {
        battle(player, true)
    } elif event <= 40 {
        battle(player)
    } elif event <= 65 {
        treasure(player)
    } elif event <= 78 {
        shrine(player)
    } elif event <= 90 {
        trap(player)
    } else {
        out "The path is quiet. You found nothing."
    }
}
func shop(player) {
    shopping = true
    while shopping {
        line()
        out f"SHOP | Your gold: {player['gold']}"
        out "1. Healing Potion - 20 gold"
        out "2. Iron Sword (+5 attack) - 80 gold"
        out "3. Steel Armor (+4 defense) - 75 gold"
        out "4. Full Heal - 35 gold"
        out "5. Leave"
        choice = input("Choose: ")
        price = 0
        if choice == 1 {
            price = 20
        } elif choice == 2 {
            if player["weapon"] == "Iron Sword" {
                out "You already own the Iron Sword!"
                continue
            }
            price = 80
        } elif choice == 3 {
            if player["armor"] == "Steel Armor" {
                out "You already own Steel Armor!"
                continue
            }
            price = 75
        } elif choice == 4 {
            price = 35
        } elif choice == 5 {
            shopping = false
            continue
        } else {
            out "Invalid option!"
            continue
        }
        if player["gold"] < price {
            out "Not enough gold!"
            continue
        }
        if choice == 4 and player["hp"] >= player["maxHp"] {
            out "Your HP is already full!"
            continue
        }
        player["gold"] -= price
        if choice == 1 {
            player["potions"] += 1
            out "Potion purchased!"
        } elif choice == 2 {
            player["attack"] += 5
            player["weapon"] = "Iron Sword"
            inventory = player["inventory"]
            inventory.push "Iron Sword"
            out "Iron Sword equipped!"
        } elif choice == 3 {
            player["defense"] += 4
            player["armor"] = "Steel Armor"
            inventory = player["inventory"]
            inventory.push "Steel Armor"
            out "Steel Armor equipped!"
        } elif choice == 4 {
            player["hp"] = player["maxHp"]
            out "Fully healed!"
        }
    }
}
func showInventory(player) {
    line()
    out "INVENTORY"
    items = player["inventory"][:]
    items.sort
    for item in items {
        out f"- {item}"
    }
    out f"Potions: {player['potions']}"
    out f"Total equipment/items: {len(items)}"
    line()
}
func saveGame(player) {
    json.save("treasureHunter_save.json", player)
    out "Game saved successfully!"
}
func loadGame() {
    if not fileExists("treasureHunter_save.json") {
        out "No saved game found!"
        return nil
    }
    loaded = json.load("treasureHunter_save.json")
    if type(loaded) != "dict" {
        out "Save file is invalid!"
        return nil
    }
    out "Saved game loaded!"
    return loaded
}
func main() {
    line()
    out "TREASURE HUNTER - RPG EDITION"
    out "Powered by FluxScript v0.7.0"
    line()
    out "1. New Game"
    out "2. Load Game"
    start = input("Choose: ")
    player = nil
    if start == 2 {
        player = loadGame()
    }
    if type(player) != "dict" {
        name = input("Enter your name: ")
        name = name.trim
        name = "Adventurer" if name == "" else name
        player = createPlayer(name)
    }
    out f"Welcome, {player['name']}!"
    playing = true
    while playing and player["hp"] > 0 {
        line()
        out "1. Explore"
        out "2. Shop"
        out "3. Inventory"
        out "4. Status"
        out "5. Use potion"
        out "6. Save Game"
        out "7. Load Game"
        out "8. Quit"
        line()
        choice = input("Choose: ")
        if choice == 1 {
            explore(player)
        } elif choice == 2 {
            shop(player)
        } elif choice == 3 {
            showInventory(player)
        } elif choice == 4 {
            showStatus(player)
        } elif choice == 5 {
            heal(player)
        } elif choice == 6 {
            saveGame(player)
        } elif choice == 7 {
            loaded = loadGame()
            if type(loaded) == "dict" {
                player = loaded
            }
        } elif choice == 8 {
            playing = false
        } else {
            out "Invalid option!"
        }
    }
    line()
    if player["hp"] <= 0 {
        out "GAME OVER! Your adventure has ended."
    } else {
        out "Thanks for playing!"
    }
    out f"Final level: {player['level']}"
    out f"Final gold: {player['gold']}"
    out f"Monsters defeated: {player['wins']}"
    out f"Bosses defeated: {player['bossWins']}"
    line()
}
try {
    main()
} catch(error) {
    out "Game error: " + error
}
