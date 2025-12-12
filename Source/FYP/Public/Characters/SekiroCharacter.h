#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// Procedural Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sekiro|Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	// Procedural Animation State
	bool bIsAttacking = false;
	float AttackTimer = 0.0f;
	
	// Base rotation for the weapon (relative to hand)
	FRotator WeaponIdleRotation;
	FRotator WeaponBlockRotation;
	FRotator WeaponAttackStartRotation;
	FRotator WeaponAttackEndRotation;

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
