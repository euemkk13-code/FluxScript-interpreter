
--[[
Dungeon Quest
A text RPG made with FluxScript v0.5.0
]]
get random

func attackDamage(level) {
    return random.randint(3, 7) + level * 2
}

func enemyDamage(power) {
    return random.randint(1, power)
}

func healPlayer(hp, maxHp) {
    amount = random.randint(15, 25)
    hp += amount

    if hp > maxHp {
        hp = maxHp
    }

    return hp
}

out "================================"
out "         DUNGEON QUEST"
out "================================"

name = input("Enter your hero name: ")

level = 1
xp = 0
gold = 0
maxHp = 40
hp = maxHp
potions = 3
floor = 1

out "Welcome, " + name + "!"
out "Defeat the dungeon boss to win!"

while hp > 0 and floor <= 5 {
    out ""
    out "================================"
    out "FLOOR:"
    out floor

    if floor == 5 {
        enemyName = "DUNGEON BOSS"
        enemyHp = 70
        enemyPower = 13
    } else {
        enemyType = random.randint(1, 3)

        if enemyType == 1 {
            enemyName = "Goblin"
            enemyHp = 15 + floor * 3
            enemyPower = 5 + floor
        } elif enemyType == 2 {
            enemyName = "Skeleton"
            enemyHp = 20 + floor * 3
            enemyPower = 6 + floor
        } else {
            enemyName = "Orc"
            enemyHp = 25 + floor * 3
            enemyPower = 7 + floor
        }
    }

    out "A wild " + enemyName + " appears!"

    while enemyHp > 0 and hp > 0 {
        out ""
        out "YOUR HP:"
        out hp
        out "ENEMY HP:"
        out enemyHp

        out "1 - Attack"
        out "2 - Use potion"
        out "3 - View stats"

        action = input("Choose an action: ")

        if action == 1 {
            damage = attackDamage(level)
            enemyHp -= damage

            out "You dealt damage:"
            out damage

        } elif action == 2 {
            if potions > 0 {
                hp = healPlayer(hp, maxHp)
                potions -= 1

                out "Potion used!"
                out "Current HP:"
                out hp
            } else {
                out "No potions left!"
                continue
            }

        } elif action == 3 {
            out "=== HERO STATS ==="
            out "Level:"
            out level
            out "XP:"
            out xp
            out "Gold:"
            out gold
            out "Potions:"
            out potions
            continue

        } else {
            out "Invalid action!"
            continue
        }

        if enemyHp <= 0 {
            break
        }

        wait(1)

        damage = enemyDamage(enemyPower)
        hp -= damage

        out enemyName + " attacks!"
        out "Damage taken:"
        out damage
    }

    if hp <= 0 {
        break
    }

    out ""
    out "You defeated " + enemyName + "!"

    if floor == 5 {
        break
    }

    gainedXp = 10 + floor * 5
    gainedGold = random.randint(5, 15)

    xp += gainedXp
    gold += gainedGold

    out "XP gained:"
    out gainedXp
    out "Gold gained:"
    out gainedGold

    if xp >= level * 20 {
        xp -= level * 20
        level += 1
        maxHp += 10
        hp = maxHp

        out "LEVEL UP!"
        out "New level:"
        out level
    }

    if random.randint(1, 3) == 1 {
        potions += 1
        out "You found a healing potion!"
    }

    floor += 1
    wait(1)
}

out ""
out "================================"

if hp <= 0 {
    out "GAME OVER!"
    out "Your hero was defeated."
} else {
    out "VICTORY!"
    out "You defeated the Dungeon Boss!"
}

out "Final level:"
out level

out "Total gold:"
out gold

out "================================"
