// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter(): LookAction(nullptr), ThrustAction(nullptr), RollAction(nullptr),
                                      MappingContext(nullptr), CurrentRollVelocity(0.0f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set up character movement
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->BrakingDecelerationFlying = 200.0f;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Interpolate towards target rotation to add inertia
	
	FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 2.5f); 
 
	// Apply roll separately
	NewRotation.Roll = GetActorRotation().Roll;
	SetActorRotation(NewRotation);

	if (FMath::Abs(CurrentRollVelocity) > 0.01f)
	{
		FRotator ActorRotation = GetActorRotation();
		ActorRotation.Roll += CurrentRollVelocity * DeltaTime;
		SetActorRotation(ActorRotation);

		// Gradually reduce roll velocity
		CurrentRollVelocity = FMath::FInterpTo(CurrentRollVelocity, 0.0f, DeltaTime, 1.0f);
	}

 	if (GEngine)
 	{
 		FString RotationString = FString::Printf(TEXT("NewRotation: Pitch: %.2f, Yaw: %.2f, Roll: %.2f"), 
 			NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll);
 		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, RotationString);
 	}
	
	// Apply inertia to roll
}

void APlayerCharacter::Look(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		// Get current actor rotation
		FRotator CurrentRotation = GetActorRotation();

		// Calculate new pitch and yaw based on input
		float PitchInput = ActionValue[1]* 10;
		float YawInput = ActionValue[0]* 10; 

		// Display PitchInput and YawInput on screen
		if (GEngine)
		{
			FString InputString = FString::Printf(TEXT("PitchInput: %.2f, YawInput: %.2f"), PitchInput, YawInput);
			GEngine->AddOnScreenDebugMessage(2, 4.0f, FColor::Green, InputString);
		}

		// Apply smoothing to the input
		static float SmoothedPitchInput = 0.0f;
		static float SmoothedYawInput = 0.0f;
		float SmoothingFactor = 0.2f;
		SmoothedPitchInput = FMath::FInterpTo(SmoothedPitchInput, PitchInput, GetWorld()->GetDeltaSeconds(), 0.5f / SmoothingFactor);
		SmoothedYawInput = FMath::FInterpTo(SmoothedYawInput, YawInput, GetWorld()->GetDeltaSeconds(), 0.5f / SmoothingFactor);

		// Get the current up vector of the character
		FVector UpVector = GetActorUpVector();

		// Calculate the right vector
		FVector RightVector = FVector::CrossProduct(GetActorForwardVector(), UpVector);

		// Create rotation axes based on the character's current orientation
		FVector PitchAxis = RightVector;
		FVector YawAxis = UpVector;

		// Apply rotations
		FQuat PitchDelta = FQuat(PitchAxis, FMath::DegreesToRadians(SmoothedPitchInput));
		FQuat YawDelta = FQuat(YawAxis, FMath::DegreesToRadians(SmoothedYawInput));

		// Combine rotations
		FQuat NewRotationQuat = YawDelta * PitchDelta * CurrentRotation.Quaternion();

		// Convert back to rotator
		TargetRotation = NewRotationQuat.Rotator().GetNormalized();
	}
}
void APlayerCharacter::Roll(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		// Calculate the roll amount
		float RollInput = ActionValue[0];
		
		// Apply roll input to current roll velocity
		float RollAcceleration = 300.0f; // Adjust this value to control roll responsiveness
		CurrentRollVelocity += RollInput * RollAcceleration * GetWorld()->GetDeltaSeconds();
		
		// Clamp the roll velocity to a maximum value
		float MaxRollVelocity = 800.0f; // Adjust this value to control maximum roll speed
		CurrentRollVelocity = FMath::Clamp(CurrentRollVelocity, -MaxRollVelocity, MaxRollVelocity);
	}
}
// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
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

void APlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Cast<APlayerController>(GetController())->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(MappingContext,0);
	}
}

void APlayerCharacter::Thrust(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		// Get the actor's current rotation
		FRotator ActorRotation = GetActorRotation();

		// Calculate thrust direction based on the actor's rotation and input
		const FVector ForwardThrust = ActorRotation.Vector() * ActionValue[0]; // X-axis input
		const FVector RightThrust = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Y) * ActionValue[1]; // Y-axis input
		const FVector UpThrust = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Z) * ActionValue[2]; // Z-axis input

		const FVector ThrustDirection = ForwardThrust + RightThrust + UpThrust;
        
		// Apply the calculated thrust direction directly to velocity
		FVector NewVelocity = GetVelocity() + ThrustDirection * 400.0f * GetWorld()->GetDeltaSeconds();
		GetCharacterMovement()->Velocity = NewVelocity;
	}
}

