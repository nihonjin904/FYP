# Coding Conventions

## Unreal Engine C++ Nomenclature
- **Prefixes**: Classes properly observe Epic's standard: `A` for Actors, `U` for Objects/Components, `E` for Enums.
- **Reflection Macros**: Heavy reliance on `UFUNCTION(BlueprintCallable)` to ensure C++ implementations can be triggered or bound within Blueprint graphs.

## UI Bridging
- C++ handles dynamic delegates and component lookups (e.g., `BindToActor`), while Blueprints define visual layouts (`BlueprintImplementableEvent` like `UpdateEnemyHealth` pushed to UMG visual graph).

## Error Handling
- Defensive programming via `if (!Owner) return;` followed up heavily with runtime logging using `GEngine->AddOnScreenDebugMessage` for visually debugging logic flow.
