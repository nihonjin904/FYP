#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FYPSaveGame.generated.h"

UCLASS()
class FYP_API UFYPSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // Names of all activated BP_Checkpoint_New actors in the level
    UPROPERTY(SaveGame)
    TArray<FString> ActivatedCheckpointNames;
};
