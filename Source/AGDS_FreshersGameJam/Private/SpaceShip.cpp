// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShip.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"


// Constructor: Initialize the spaceship with default values and set up movement
ASpaceShip::ASpaceShip(): LookAction(nullptr), ThrustAction(nullptr), RollAction(nullptr),
                          MappingContext(nullptr), CurrentRollVelocity(0.0f), bIsLanded(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and initialize the floating pawn movement component
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->SetUpdatedComponent(GetRootComponent());
	FloatingPawnMovement->MaxSpeed = 1000.0f;
	FloatingPawnMovement->Acceleration = 400.0f;
	FloatingPawnMovement->Deceleration = 200.0f;

	// Create a capsule component for collision detection
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetCollisionProfileName(TEXT("Pawn"));
}

void ASpaceShip::BeginPlay()
{
	Super::BeginPlay();
}

void ASpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateRotation(DeltaTime);
	UpdateSmoothInput(DeltaTime);
	CalculateTargetRotation();
	CheckForLanding();
}

void ASpaceShip::UpdateRotation(float DeltaTime)
{
	if (!bIsLanded)
	{
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 2.5f);
		
		if (!FMath::IsNearlyZero(CurrentRollVelocity, 0.1f))
		{
			NewRotation.Roll += CurrentRollVelocity * DeltaTime;
			CurrentRollVelocity = FMath::FInterpTo(CurrentRollVelocity, 0.0f, DeltaTime, 1.0f);
		}
		SetActorRotation(NewRotation);
	}
}

void ASpaceShip::UpdateSmoothInput(float DeltaTime)
{
	const float SmoothingFactor = 0.2f;
	const float InterpSpeed = 0.5f / SmoothingFactor;

	SmoothedPitchInput = FMath::FInterpTo(SmoothedPitchInput, PitchInput, DeltaTime, InterpSpeed);
	SmoothedYawInput = FMath::FInterpTo(SmoothedYawInput, YawInput, DeltaTime, InterpSpeed);

	PitchInput = 0.0f;
	YawInput = 0.0f;
}

void ASpaceShip::CalculateTargetRotation()
{
	FRotator CurrentRotation = GetActorRotation();
	FVector UpVector = GetActorUpVector();
	FVector RightVector = FVector::CrossProduct(GetActorForwardVector(), UpVector);

	FQuat PitchDelta = FQuat(RightVector, FMath::DegreesToRadians(SmoothedPitchInput));
	FQuat YawDelta = FQuat(UpVector, FMath::DegreesToRadians(SmoothedYawInput));

	FQuat NewRotationQuat = YawDelta * PitchDelta * CurrentRotation.Quaternion();
	TargetRotation = NewRotationQuat.Rotator();
}

void ASpaceShip::Look(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr && !bIsLanded)
	{
		if (ActionValue.GetValueType() == EInputActionValueType::Axis2D)
		{
			FVector2D LookAxisVector = ActionValue.Get<FVector2D>();
			PitchInput = LookAxisVector.Y * 10;
			YawInput = LookAxisVector.X * 10;
		}
	}
}

void ASpaceShip::Roll(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr && !bIsLanded)
	{
		float RollInput = ActionValue[0];
		
		float RollAcceleration = 300.0f;
		CurrentRollVelocity += RollInput * RollAcceleration * GetWorld()->GetDeltaSeconds();
		
		float MaxRollVelocity = 800.0f;
		CurrentRollVelocity = FMath::Clamp(CurrentRollVelocity, -MaxRollVelocity, MaxRollVelocity);
	}
}

void ASpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ThrustAction) 
		{
			PlayerEnhancedInputComponent->BindAction(ThrustAction, ETriggerEvent::Triggered, this, &ASpaceShip::Thrust);
		}
		if (LookAction)
		{
			PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceShip::Look);
		}
		if (RollAction)
		{
			PlayerEnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASpaceShip::Roll);
		}
	}
}

void ASpaceShip::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = Cast<APlayerController>(GetController())->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(MappingContext,0);
	}
}

void ASpaceShip::Thrust(const FInputActionValue& ActionValue)
{
	if (Controller != nullptr)
	{
		if (bIsLanded)
		{
			TakeOff();
		}
		else
		{
			FRotator ActorRotation = GetActorRotation();

			const FVector ForwardThrust = ActorRotation.Vector() * ActionValue[0];
			const FVector RightThrust = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Y) * ActionValue[1];
			const FVector UpThrust = FRotationMatrix(ActorRotation).GetUnitAxis(EAxis::Z) * ActionValue[2] * 5;

			const FVector ThrustDirection = ForwardThrust + RightThrust + UpThrust;
			
			FVector NewVelocity = GetVelocity() + ThrustDirection * 4000.0f * GetWorld()->GetDeltaSeconds();
			FloatingPawnMovement->Velocity = NewVelocity;
		}
	}
}

void ASpaceShip::CheckForLanding()
{
	if (!bIsLanded)
	{
		FVector Start = GetActorLocation();
		FVector End = Start -  FRotationMatrix(GetActorRotation()).GetUnitAxis(EAxis::Z) * 500.0f;
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			if (FloatingPawnMovement->Velocity.SizeSquared() < 10000.0f) // Velocity less than 100 units/s
			{
				Land(HitResult.ImpactPoint, HitResult.ImpactNormal);
			}
		}
	}
}

void ASpaceShip::Land(const FVector& LandingPoint, const FVector& SurfaceNormal)
{
	bIsLanded = true;
	FloatingPawnMovement->StopMovementImmediately();
	FloatingPawnMovement->SetActive(false);

	FRotator LandingRotation = FRotationMatrix::MakeFromZ(SurfaceNormal).Rotator();
	SetActorLocationAndRotation(LandingPoint + SurfaceNormal * 100.0f, LandingRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ASpaceShip::TakeOff()
{
	bIsLanded = false;
	FloatingPawnMovement->SetActive(true);
	AddActorWorldOffset(FVector::UpVector * 100.0f, false, nullptr, ETeleportType::TeleportPhysics);
}
