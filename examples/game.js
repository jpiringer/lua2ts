let Player = { name: "Hero", hp: 100, inventory: ["sword", "shield", "potion"] };
Player.takeDamage = function (amount) {
    const self = this;
    self.hp = (self.hp - amount);
    if ((self.hp <= 0)) {
        console.log((self.name + " has fallen in battle!"));
    }
    else {
        console.log((self.name + (" took " + (amount + (" damage, remaining HP: " + self.hp)))));
    }
};
Player.heal = function (amount) {
    const self = this;
    self.hp = Math.min(100, (self.hp + amount));
    console.log((self.name + (" healed to " + (self.hp + " HP"))));
};
function simulateBattle() {
    console.log("Battle begins!");
    for (let round = 1; round <= 3; round++) {
        let dmg = Math.floor((15 + (5 * round)));
        Player.takeDamage(dmg);
    }
    console.log("Checking inventory:");
    for (const [_i_idx, item] of Player.inventory.entries()) {
        const i = _i_idx + 1;
        console.log(("Slot " + (i + (": " + item))));
    }
}
simulateBattle();
