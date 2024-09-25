// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "PlanetGravity.generated.h"

UCLASS()
class APlanetGravity : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanetGravity();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereCollider;

	UPROPERTY(EditAnywhere)
	float GravityStrength;

	double GravitationalConstant;
	UPROPERTY(EditAnywhere)
	double PlanetMass;

};
