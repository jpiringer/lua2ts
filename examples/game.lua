-- Example Lua game logic

local Player = {
    name = "Hero",
    hp = 100,
    inventory = { "sword", "shield", "potion" }
}

function Player:takeDamage(amount)
    self.hp = self.hp - amount
    if self.hp <= 0 then
        print(self.name .. " has fallen in battle!")
    else
        print(self.name .. " took " .. amount .. " damage, remaining HP: " .. self.hp)
    end
end

function Player:heal(amount)
    self.hp = math.min(100, self.hp + amount)
    print(self.name .. " healed to " .. self.hp .. " HP")
end

local function simulateBattle()
    print("Battle begins!")
    for round = 1, 3 do
        local dmg = math.floor(15 + 5 * round)
        Player:takeDamage(dmg)
    end

    print("Checking inventory:")
    for i, item in ipairs(Player.inventory) do
        print("Slot " .. i .. ": " .. item)
    end
end

simulateBattle()
