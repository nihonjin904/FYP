#include "Characters/SekiroCharacter.h"
#include "Components/SekiroPostureComponent.h"
#include "Components/SekiroDeflectComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"

ASekiroCharacter::ASekiroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	// Create Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 

	// Create Combat Components
	PostureComponent = CreateDefaultSubobject<USekiroPostureComponent>(TEXT("PostureComponent"));
	DeflectComponent = CreateDefaultSubobject<USekiroDeflectComponent>(TEXT("DeflectComponent"));
	CombatComponent = CreateDefaultSubobject<USekiroCombatComponent>(TEXT("CombatComponent"));
}

void ASekiroCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ASekiroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASekiroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASekiroCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASekiroCharacter::Look);

		// Blocking
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &ASekiroCharacter::StartBlock);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &ASekiroCharacter::StopBlock);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASekiroCharacter::Attack);
	}
}

void ASekiroCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASekiroCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASekiroCharacter::StartBlock()
{
	if (DeflectComponent)
	{
		DeflectComponent->StartBlocking();
	}
	Tags.Add(FName("State.Combat.HoldingBlock"));
}

void ASekiroCharacter::StopBlock()
{
	if (DeflectComponent)
	{
		DeflectComponent->StopBlocking();
	}
	Tags.Remove(FName("State.Combat.HoldingBlock"));
}

void ASekiroCharacter::Attack()
{
	if (CombatComponent)
	{
		CombatComponent->RequestAttack();
	}
}
