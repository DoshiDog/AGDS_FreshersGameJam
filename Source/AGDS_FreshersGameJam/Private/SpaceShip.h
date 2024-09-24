// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SpaceShip.generated.h"

UCLASS()
class ASpaceShip : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASpaceShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Input actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* ThrustAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* MappingContext;

private:
	float CurrentRollVelocity;
	float PitchInput;
	float YawInput;
	float SmoothedPitchInput;
	float SmoothedYawInput;
	FRotator TargetRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* FloatingPawnMovement;

	void UpdateRotation(float DeltaTime);
	void UpdateSmoothInput(float DeltaTime);
	void CalculateTargetRotation();
	void Look(const FInputActionValue& ActionValue);
	void Roll(const FInputActionValue& ActionValue);
	void Thrust(const FInputActionValue& ActionValue);
	void PawnClientRestart();
};