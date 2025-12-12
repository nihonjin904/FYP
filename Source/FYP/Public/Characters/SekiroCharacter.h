#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SekiroCharacter.generated.h"

class USekiroPostureComponent;
class USekiroDeflectComponent;
class USekiroCombatComponent;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

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

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartBlock();
	void StopBlock();
	void Attack();
};
