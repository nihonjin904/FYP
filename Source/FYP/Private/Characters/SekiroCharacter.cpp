 
#include "Characters/SekiroCharacter.h"
#include "Components/SekiroPostureComponent.h"
#include "Components/SekiroDeflectComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/WidgetComponent.h"
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
	AttributeComponent = CreateDefaultSubobject<USekiroAttributeComponent>(TEXT("AttributeComponent"));

	// Create Overhead Widget
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadWidget->SetDrawAtDesiredSize(true);
	OverheadWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Above head

	// Create Deathblow Widget
	DeathblowWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DeathblowWidget"));
	DeathblowWidget->SetupAttachment(RootComponent);
	DeathblowWidget->SetWidgetSpace(EWidgetSpace::Screen);
	DeathblowWidget->SetDrawAtDesiredSize(true);
	DeathblowWidget->SetVisibility(false); // Hidden by default
	DeathblowWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f)); // Chest/Head level

	// Create Weapon Mesh
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), FName("hand_r")); // Attach to right hand socket if available
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Visual only

	// Initialize Rotations
	WeaponIdleRotation = FRotator(0.0f, 0.0f, 0.0f);
	WeaponBlockRotation = FRotator(0.0f, 90.0f, 45.0f); // Horizontal block
	WeaponAttackStartRotation = FRotator(-45.0f, 0.0f, 0.0f); // Windup
	WeaponAttackEndRotation = FRotator(60.0f, 0.0f, 0.0f); // Swing down

	// Default Weapon Mesh (Cube)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		WeaponMesh->SetStaticMesh(CubeMeshAsset.Object);
		WeaponMesh->SetWorldScale3D(FVector(0.1f, 0.1f, 1.0f)); // Make it look like a sword blade
	}
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

	if (PostureComponent)
	{
		PostureComponent->OnPostureBroken.AddDynamic(this, &ASekiroCharacter::OnPostureBroken);
	}
}

void ASekiroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!WeaponMesh) return;

	FRotator TargetRotation = WeaponIdleRotation;

	// 1. Attack Priority
	if (bIsAttacking)
	{
		AttackTimer += DeltaTime;
		if (AttackTimer < 0.1f) // Windup
		{
			TargetRotation = WeaponAttackStartRotation;
		}
		else if (AttackTimer < 0.3f) // Swing
		{
			float Alpha = (AttackTimer - 0.1f) / 0.2f;
			TargetRotation = FMath::Lerp(WeaponAttackStartRotation, WeaponAttackEndRotation, Alpha);
		}
		else // Recovery
		{
			bIsAttacking = false;
		}
	}
	// 2. Block Priority
	else if (DeflectComponent && DeflectComponent->IsBlocking())
	{
		TargetRotation = WeaponBlockRotation;
	}

	// Interpolate to target
	FRotator NewRotation = FMath::RInterpTo(WeaponMesh->GetRelativeRotation(), TargetRotation, DeltaTime, 15.0f);
	WeaponMesh->SetRelativeRotation(NewRotation);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASekiroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
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
		
		// Execution
		EnhancedInputComponent->BindAction(ExecutionAction, ETriggerEvent::Started, this, &ASekiroCharacter::Execution);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASekiroCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASekiroCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASekiroCharacter::StartBlock()
{
	if (DeflectComponent)
	{
		DeflectComponent->StartBlocking();
		// Tag is handled by component, visual is handled by Tick
	}
}

void ASekiroCharacter::StopBlock()
{
	if (DeflectComponent)
	{
		DeflectComponent->StopBlocking();
	}
}

void ASekiroCharacter::Attack()
{
	if (CombatComponent)
	{
		CombatComponent->RequestAttack();
		
		// Trigger Procedural Animation
		bIsAttacking = true;
		AttackTimer = 0.0f;
	}
}

void ASekiroCharacter::Execution(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestExecution();
	}
}

void ASekiroCharacter::OnPostureBroken()
{
	if (DeathblowWidget)
	{
		DeathblowWidget->SetVisibility(true);
	}

	// Add Stunned Tag
	Tags.Add(FName("State.Stunned"));

	// Optional: Disable movement or AI logic here
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}
