
#include "Characters/SekiroCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SekiroAttributeComponent.h"
#include "Components/SekiroCombatComponent.h"
#include "Components/SekiroDeflectComponent.h"
#include "Components/SekiroPostureComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayTagContainer.h"


ASekiroCharacter::ASekiroCharacter() {
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
  PostureComponent =
      CreateDefaultSubobject<USekiroPostureComponent>(TEXT("PostureComponent"));
  DeflectComponent =
      CreateDefaultSubobject<USekiroDeflectComponent>(TEXT("DeflectComponent"));
  CombatComponent =
      CreateDefaultSubobject<USekiroCombatComponent>(TEXT("CombatComponent"));
  AttributeComponent = CreateDefaultSubobject<USekiroAttributeComponent>(
      TEXT("AttributeComponent"));

  // Create Overhead Widget
  OverheadWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
  OverheadWidget->SetupAttachment(RootComponent);
  OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
  OverheadWidget->SetDrawAtDesiredSize(true);
  OverheadWidget->SetRelativeLocation(
      FVector(0.0f, 0.0f, 100.0f)); // Above head

  // Create Deathblow Widget
  DeathblowWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("DeathblowWidget"));
  DeathblowWidget->SetupAttachment(RootComponent);
  DeathblowWidget->SetWidgetSpace(EWidgetSpace::Screen);
  DeathblowWidget->SetDrawAtDesiredSize(true);
  DeathblowWidget->SetVisibility(false); // Hidden by default
  DeathblowWidget->SetRelativeLocation(
      FVector(0.0f, 0.0f, 50.0f)); // Chest/Head level

  // Create Weapon Mesh
  WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
  WeaponMesh->SetupAttachment(
      GetMesh(), FName("hand_r")); // Attach to right hand socket if available
  WeaponMesh->SetCollisionEnabled(
      ECollisionEnabled::NoCollision); // Visual only

  // Default Weapon Mesh (Cube)
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(
      TEXT("/Engine/BasicShapes/Cube.Cube"));
  if (CubeMeshAsset.Succeeded()) {
    WeaponMesh->SetStaticMesh(CubeMeshAsset.Object);
    WeaponMesh->SetWorldScale3D(
        FVector(0.1f, 0.1f, 1.0f)); // Make it look like a sword blade
  }
}

void ASekiroCharacter::BeginPlay() {
  Super::BeginPlay();

  // Add Input Mapping Context
  if (APlayerController *PlayerController =
          Cast<APlayerController>(Controller)) {
    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PlayerController->GetLocalPlayer())) {
      if (DefaultMappingContext) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
      }
    }
  }

  if (PostureComponent) {
    PostureComponent->OnPostureBroken.AddDynamic(
        this, &ASekiroCharacter::OnPostureBroken);
  }

  if (DeflectComponent) {
    DeflectComponent->OnParryResult.AddDynamic(
        this, &ASekiroCharacter::HandleParryResult);
  }

  if (AttributeComponent) {
    AttributeComponent->OnDeath.AddDynamic(this, &ASekiroCharacter::OnDeath);
  }
}

void ASekiroCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(
        10, 0.f, bIsBlocking ? FColor::Green : FColor::Red,
        FString::Printf(TEXT("bIsBlocking: %s"),
                        bIsBlocking ? TEXT("TRUE") : TEXT("FALSE")));
  }
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASekiroCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

    // Jumping
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
                                       &ACharacter::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
                                       this, &ACharacter::StopJumping);

    // Moving
    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                       this, &ASekiroCharacter::Move);

    // Looking
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                       this, &ASekiroCharacter::Look);

    // Blocking
    if (BlockAction) {
      UE_LOG(LogTemp, Warning, TEXT("Binding BlockAction: %s"),
             *BlockAction->GetName());
      EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started,
                                         this, &ASekiroCharacter::StartBlock);
      EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed,
                                         this, &ASekiroCharacter::StopBlock);
    } else {
      UE_LOG(LogTemp, Error,
             TEXT("BlockAction is NULL in SetupPlayerInputComponent!"));
    }

    // Attacking
    EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started,
                                       this, &ASekiroCharacter::Attack);

    // Execution
    EnhancedInputComponent->BindAction(ExecutionAction, ETriggerEvent::Started,
                                       this, &ASekiroCharacter::Execution);
  } else {
    UE_LOG(
        LogTemp, Error,
        TEXT("'%s' Failed to find an Enhanced Input component! This template "
             "is built to use the Enhanced Input system. If you intend to use "
             "the legacy system, then you will need to update this C++ file."),
        *GetNameSafe(this));
  }
}

void ASekiroCharacter::Move(const FInputActionValue &Value) {
  // Lock movement if any montage is playing (Attack, Hit, Parry, etc.)
  if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
    return;

  // input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // find out which way is forward
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    // get forward vector
    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // get right vector
    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // add movement
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
  }
}

void ASekiroCharacter::Look(const FInputActionValue &Value) {
  // input is a Vector2D
  FVector2D LookAxisVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // add yaw and pitch input to controller
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
  }
}

void ASekiroCharacter::StartBlock() {
  UE_LOG(LogTemp, Warning, TEXT("StartBlock Called (Log)!"));
  if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
                                     TEXT("StartBlock Called!"));

  if (DeflectComponent) {
    UE_LOG(LogTemp, Warning,
           TEXT("DeflectComponent is Valid. Setting bIsBlocking = true"));
    DeflectComponent->StartBlocking();
    bIsBlocking = true;

    // Play Parry Attempt (Fast guard up)
    if (ParryAttemptMontage) {
      PlayAnimMontage(ParryAttemptMontage);
    }
  } else {
    UE_LOG(LogTemp, Error,
           TEXT("DeflectComponent is NULL! bIsBlocking NOT set."));
    if (GEngine)
      GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                       TEXT("DeflectComponent is NULL!"));
  }
}

void ASekiroCharacter::StopBlock() {
  UE_LOG(LogTemp, Warning, TEXT("StopBlock Called (Log)!"));
  if (GEngine)
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
                                     TEXT("StopBlock Called!"));

  if (DeflectComponent) {
    DeflectComponent->StopBlocking();
    bIsBlocking = false;
  }
}

void ASekiroCharacter::Attack() {
  if (CombatComponent) {
    CombatComponent->RequestAttack();
  }
}

void ASekiroCharacter::Execution(const FInputActionValue &Value) {
  if (CombatComponent) {
    CombatComponent->RequestExecution();

    if (ExecutionMontage) {
      PlayAnimMontage(ExecutionMontage);
    } else {
      if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
                                         TEXT("ExecutionMontage is NULL!"));
    }
  }
}

void ASekiroCharacter::OnPostureBroken() {
  if (DeathblowWidget) {
    DeathblowWidget->SetVisibility(true);
  }

  // Add Stunned Tag
  Tags.Add(FName("State.Stunned"));

  // Optional: Disable movement or AI logic here
  if (GetCharacterMovement()) {
    GetCharacterMovement()->StopMovementImmediately();
  }

  if (StunMontage) {
    PlayAnimMontage(StunMontage);
  }
}

void ASekiroCharacter::HandleParryResult(EParryResult Result) {
  switch (Result) {
  case EParryResult::Perfect:
    if (ParrySuccessMontage) {
      PlayAnimMontage(ParrySuccessMontage);
    }
    break;
  case EParryResult::Blocked:
    if (BlockHitMontage) {
      PlayAnimMontage(BlockHitMontage);
    }
    break;
  case EParryResult::Failed:
    if (HitMontage) {
      PlayAnimMontage(HitMontage);
    }
    break;
  }
}

void ASekiroCharacter::OnDeath() {
  if (GetCharacterMovement()) {
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
  }

  if (Controller) {
    DisableInput(Cast<APlayerController>(Controller));
  }

  if (DeathMontage) {
    PlayAnimMontage(DeathMontage);
  }
}
