#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/SekiroDeflectComponent.h"
#include "SekiroCharacter.generated.h"

class USekiroPostureComponent;
class USekiroDeflectComponent;
class USekiroCombatComponent;
class USekiroAttributeComponent;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class FYP_API ASekiroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASekiroCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
	TObjectPtr<USekiroPostureComponent> PostureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
	TObjectPtr<USekiroDeflectComponent> DeflectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
	TObjectPtr<USekiroCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Components")
	TObjectPtr<USekiroAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> OverheadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> DeathblowWidget;

	UFUNCTION()
	void OnPostureBroken();

	UFUNCTION()
	void HandleParryResult(EParryResult Result);

	UFUNCTION()
	void OnDeath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// Procedural Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// Animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> ParryAttemptMontage; // Press Block (0.5s)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> ParrySuccessMontage; // Perfect Parry spark

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> BlockHitMontage; // Blocked a hit

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> HitMontage; // Failed parry (Take damage)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> ExecutionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> StunMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|State")
	bool bIsBlocking = false;

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputAction> BlockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sekiro Input")
	TObjectPtr<UInputAction> ExecutionAction;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartBlock();
	void StopBlock();
	void Attack();
	void Execution(const FInputActionValue& Value);
};
