// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGP_2526Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MGP_2526.h"
#include "Kismet/GameplayStatics.h"
#include "ClashEnemy.h"
#include "ClashManager.h"
#include "InputActionValue.h"



AMGP_2526Character::AMGP_2526Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMGP_2526Character::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext) // load inputs
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMGP_2526Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::Look);

		
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMGP_2526Character::TryInitiateClash);

		//Indiviual bindings for each key
		EnhancedInputComponent->BindAction(ClashKeyQ, ETriggerEvent::Started, this, &AMGP_2526Character::PressQ);
		EnhancedInputComponent->BindAction(ClashKeyW, ETriggerEvent::Started, this, &AMGP_2526Character::PressW);
		EnhancedInputComponent->BindAction(ClashKeyE, ETriggerEvent::Started, this, &AMGP_2526Character::PressE);
		EnhancedInputComponent->BindAction(ClashKeyR, ETriggerEvent::Started, this, &AMGP_2526Character::PressR);
		EnhancedInputComponent->BindAction(ClashKeyF, ETriggerEvent::Started, this, &AMGP_2526Character::PressF);
		EnhancedInputComponent->BindAction(ClashKeyG, ETriggerEvent::Started, this, &AMGP_2526Character::PressG);

		
	}
	else
	{
		UE_LOG(LogMGP_2526, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMGP_2526Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMGP_2526Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMGP_2526Character::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMGP_2526Character::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMGP_2526Character::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AMGP_2526Character::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

AClashEnemy* AMGP_2526Character::FindNearbyEnemy()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AClashEnemy::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AClashEnemy* Enemy = Cast<AClashEnemy>(Actor); // locate Clash Enemy
		if (Enemy && Enemy->bCanClash)// if enemy can clash
		{
			float Distance = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
			if (Distance <= Enemy->InteractionRange) // if enemy is in range to clash
			{
				return Enemy;
			}
		}
	}
	return nullptr;
}

void AMGP_2526Character::TryInitiateClash(const FInputActionValue& Value)
{
	AClashEnemy* NearbyEnemy = FindNearbyEnemy();
	if (NearbyEnemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clash initiated!"));

		NearbyEnemy->bCanClash = false; //prevent multiple clashes with same enemy

		// Find the ClashManager in the world and start the clash
		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
		AClashManager* Manager = Cast<AClashManager>(FoundActor);
		Manager->StartClash(NearbyEnemy);
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No enemy nearby to clash with."));
	}
}


//list of each key method when player inputs
void AMGP_2526Character::PressQ(const FInputActionValue& Value)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
	AClashManager* Manager = Cast<AClashManager>(FoundActor);
	if (Manager) Manager->OnPlayerInput(EKeys::Q);
}

void AMGP_2526Character::PressW(const FInputActionValue& Value)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
	AClashManager* Manager = Cast<AClashManager>(FoundActor);
	if (Manager) Manager->OnPlayerInput(EKeys::W);
}

void AMGP_2526Character::PressE(const FInputActionValue& Value)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
	AClashManager* Manager = Cast<AClashManager>(FoundActor);
	if (Manager) Manager->OnPlayerInput(EKeys::E);
}

void AMGP_2526Character::PressR(const FInputActionValue& Value)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
	AClashManager* Manager = Cast<AClashManager>(FoundActor);
	if (Manager) Manager->OnPlayerInput(EKeys::R);
}

void AMGP_2526Character::PressF(const FInputActionValue& Value)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
	AClashManager* Manager = Cast<AClashManager>(FoundActor);
	if (Manager) Manager->OnPlayerInput(EKeys::F);
}

void AMGP_2526Character::PressG(const FInputActionValue& Value)
{
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AClashManager::StaticClass());
	AClashManager* Manager = Cast<AClashManager>(FoundActor);
	if (Manager) Manager->OnPlayerInput(EKeys::G);
}
