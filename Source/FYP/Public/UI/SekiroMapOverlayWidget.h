#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SekiroMapOverlayWidget.generated.h"

class UImage;
class UCanvasPanel;
class UCanvasPanelSlot;
class UTextBlock;
class UButton;
class UVerticalBox;
class UBorder;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class ASekiroCharacter;
class UWidgetTree;

/**
 * Full-screen map overlay widget.
 * Displays RT_Minimap texture and dynamically spawns checkpoint icons
 * with speech bubble labels. Supports click-to-select and F-to-teleport.
 */
UCLASS()
class FYP_API USekiroMapOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ===== BindWidgetOptional: only MapImage needs to exist in the BP =====
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* MapImage = nullptr;

    // Optional prompt text — C++ handles null gracefully
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* TeleportPrompt = nullptr;

    // Must match ASekiroCharacter SceneCapture OrthoWidth (default 3000)
    UPROPERTY(EditDefaultsOnly, Category = "Map")
    float MapOrthoWidth = 3000.f;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }

private:
    struct FCheckpointIconInfo
    {
        FString ActorName;
        FVector WorldPos;
        bool bActivated = false;
        TWeakObjectPtr<AActor> Actor;
        UButton* DotButton = nullptr;
    };

    TArray<FCheckpointIconInfo> CheckpointIcons;
    int32 SelectedCheckpointIndex = -1;
    float MapZoom = 1.0f;
    bool  bDragging = false;
    bool  bIconsBuilt = false;
    FVector2D LastMousePos = FVector2D::ZeroVector;
    TArray<TWeakObjectPtr<UWidget>> DynamicMapIcons;  // for cleanup on rebuild
    TArray<UCanvasPanelSlot*>       DynamicIconSlots; // for direct translation during drag

    void BuildCheckpointIcons();
    void SelectCheckpoint(int32 Index);

    UFUNCTION()
    void OnAnyCheckpointClicked();
};
