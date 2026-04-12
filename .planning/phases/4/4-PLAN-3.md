---
wave: 3
depends_on: [".planning/phases/4/4-PLAN-2.md"]
files_modified: []
autonomous: true
---

# Wave 3: Additive Attribute Modifiers

<task>
<read_first>
- .planning/phases/4/4-CONTEXT.md
</read_first>
<action>
1. Locate the existing `AttributeComponent` on the player character.
2. Add three internal float variables: `BonusHealth`, `BonusAttack`, `BonusPostureMitigation` initialized to 0.
3. Update the core Getters (e.g., `GetMaxHealth()`, `GetAttackDamage()`) to dynamically return their original serialized `BaseValue` plus the new `Bonus` variables. Ensure no permanent modification of the Base variables occurs.
4. Add three new functions to the Attribute Component: `UpgradeHealth()`, `UpgradeAttack()`, `UpgradePosture()`. These functions simply increment the respective `Bonus` variables (e.g., `BonusHealth += 10.0f;`).
5. Link the OnClicked events of the three buttons in `WBP_UpgradeMenu` to cast to the Player Character's Attribute Component and call these new Upgrade functions.
</action>
<acceptance_criteria>
- The existing Attribute Component cleanly houses internal `Bonus` modifiers safely isolated from `Base` variables.
- Clicking the UI buttons directly increments these modifiers.
- The player experiences the stat boosts without corrupting base serialized default variables.
</acceptance_criteria>
</task>
