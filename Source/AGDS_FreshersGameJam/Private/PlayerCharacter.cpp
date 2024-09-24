#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

// Constructor: Initialize the player character with default values and set up movement
APlayerCharacter::APlayerCharacter(): LookAction(nullptr), ThrustAction(nullptr), RollAction(nullptr),
                                      MappingContext(nullptr), CurrentRollVelocity(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;

	// Set the character to flying mode for space-like movement
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->BrakingDecelerationFlying = 200.0f;
}

void APlayerCharacter::BeginPlay() {Super::BeginPlay();}

// Main update function called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateRotation(DeltaTime);
	UpdateSmoothInput(DeltaTime);
	CalculateTargetRotation();
}

// Smoothly interpolate the character's rotation towards the target rotation
void APlayerCharacter::UpdateRotation(float DeltaTime)
{
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 2.5f);
	
	// Apply roll velocity if it's significant
	if (!FMath::IsNearlyZero(CurrentRollVelocity, 0.1f))
	{
		NewRotation.Roll += CurrentRollVelocity * DeltaTime;
		CurrentRollVelocity = FMath::FInterpTo(CurrentRollVelocity, 0.0f, DeltaTime, 1.0f);
	}
	SetActorRotation(NewRotation);
}

// Smooth out input values for more fluid movement
void APlayerCharacter::UpdateSmoothInput(float DeltaTime)
{
	const float SmoothingFactor = 0.2f;
	const float InterpSpeed = 0.5f / SmoothingFactor;

	SmoothedPitchInput = FMath::FInterpTo(SmoothedPitchInput, PitchInput, DeltaTime, InterpSpeed);
	SmoothedYawInput = FMath::FInterpTo(SmoothedYawInput, YawInput, DeltaTime, InterpSpeed);

	// Reset input values after smoothing
	PitchInput = 0.0f;
	YawInput = 0.0f;
}

// Calculate the target rotation based on smoothed input
void APlayerCharacter::CalculateTargetRotation()
{
	FRotator CurrentRotation = GetActorRotation();
	FVector UpVector = GetActorUpVector();
	FVector RightVector = FVector::CrossProduct(GetActorForwardVector(), UpVector);

	// Create quaternions for pitch and yaw rotations
	FQuat PitchDelta = FQuat(RightVector, FMath::DegreesToRadians(SmoothedPitchInput));
	FQuat YawDelta = FQuat(UpVector, FMath::DegreesToRadians(SmoothedYawInput));

	// Combine rotations and convert back to FRotator
	FQuat NewRotationQuat = YawDelta * PitchDelta * CurrentRotation.Quaternion();
	TargetRotation = NewRotationQuat.Rotator();
}

// Handle look input (pitch and yaw)
void APlayerCharacter::Look(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		if (ActionValue.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D LookAxisVector = ActionValue.Get<FVector2D>();
			PitchInput = LookAxisVector.Y * 10;
			YawInput = LookAxisVector.X * 10;
		}
	}
}

// Handle roll input
void APlayerCharacter::Roll(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		float RollInput = ActionValue[0];
		
		// Apply roll acceleration
		float RollAcceleration = 300.0f;
		CurrentRollVelocity += RollInput * RollAcceleration * GetWorld()->GetDeltaSeconds();
		
		// Clamp roll velocity to prevent excessive rolling
		float MaxRollVelocity = 800.0f;
		CurrentRollVelocity = FMath::Clamp(CurrentRollVelocity, -MaxRollVelocity, MaxRollVelocity);
	}
}

// Set up input bindings for the player
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind actions to their respective functions
		if (ThrustAction) 
		{
			PlayerEnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Thrust);
		}
		if (LookAction)
		{
			PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		}
		if (RollAction)
		{
			PlayerEnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Roll);
		}
	}
}

// Set up input mapping context when the pawn restarts
void APlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Cast<APlayerController>(GetController())->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(MappingContext,0);
	}
}

// Handle thrust input for 3D movement
void APlayerCharacter::Thrust(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		FRotator ActorRotation = GetActorRotation();

		// Calculate thrust vectors for each direction
		const FVector ForwardThrust = ActorRotation.Vector() * ActionValue[0];
		const FVector RightThrust = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Y) * ActionValue[1];
		const FVector UpThrust = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Z) * ActionValue[2];

		// Combine thrust vectors
		const FVector ThrustDirection = ForwardThrust + RightThrust + UpThrust;
        
		// Apply thrust to character's velocity
		FVector NewVelocity = GetVelocity() + ThrustDirection * 400.0f * GetWorld()->GetDeltaSeconds();
		GetCharacterMovement()->Velocity = NewVelocity;
	}
}
