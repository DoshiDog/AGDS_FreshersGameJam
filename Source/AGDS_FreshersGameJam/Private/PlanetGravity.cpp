// Fill out your copyright notice in the Description page of Project Settings.

#include "PlanetGravity.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlanetGravity::APlanetGravity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a sphere component to represent the planet
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(100.0f);
	SphereComponent->SetCollisionProfileName(TEXT("YourCollisionProfile")); // Set a collision profile

	// Set other collision-related properties as needed
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	
	// Set default pseudo gravity strength
	GravityStrength = 9.8f * 50; // Negative value for attractive force
}

// Called when the game starts or when spawned
void APlanetGravity::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlanetGravity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TArray<AActor*> ArrayOverlappingActors;
	GetOverlappingActors(ArrayOverlappingActors);
    if (ArrayOverlappingActors.Num() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Overlapping Component: None"));
    }
	for (AActor* OverlappingActor : ArrayOverlappingActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlapping Component: %s"), *OverlappingActor->GetName());

		if (UPrimitiveComponent* OverlappingComponent = Cast<UPrimitiveComponent>(OverlappingActor->GetRootComponent()))
		{
			// Calculate the direction towards the planet's center
			FVector DirectionToPlanet = GetActorLocation() - OverlappingComponent->GetComponentLocation();
			DirectionToPlanet.Normalize();

			// Calculate pseudo gravity force
			FVector PseudoGravityForce = DirectionToPlanet * GravityStrength;
			UE_LOG(LogTemp, Warning, TEXT("Overlapping Component: %s"), *PseudoGravityForce.ToString());
			// Apply the pseudo gravitational force or velocity
			APawn* Pawn = Cast<APawn>(OverlappingActor);
			if (Pawn)
			{
				// For Pawns, apply velocity to FloatingPawnMovement
				UFloatingPawnMovement* FloatingPawnMovement = Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent());
				if (FloatingPawnMovement)
				{
					FVector CurrentVelocity = FloatingPawnMovement->Velocity;
					FVector NewVelocity = CurrentVelocity + PseudoGravityForce * DeltaTime;
					
					FloatingPawnMovement->Velocity = NewVelocity;

					// Apply subtle orientation effect
					FVector UpVector = -DirectionToPlanet;
					FRotator TargetRotation = UKismetMathLibrary::MakeRotFromZX(UpVector, Pawn->GetActorForwardVector());
					FRotator NewRotation = FMath::RInterpTo(Pawn->GetActorRotation(), TargetRotation, DeltaTime, 0.05f);
					Pawn->SetActorRotation(NewRotation);
				}
			}
			else if (OverlappingComponent->IsSimulatingPhysics())
			{
				// For non-Pawn physics objects, apply force
				OverlappingComponent->AddForce(PseudoGravityForce, NAME_None, true);
			}
		}
	}
}
