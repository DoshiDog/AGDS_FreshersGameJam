// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();
	
    UPROPERTY(EditDefaultsOnly, Category = "KeyBinds")
    UInputAction* LookAction;
    UPROPERTY(EditDefaultsOnly, Category = "KeyBinds")
    UInputAction* MoveAction;
    UPROPERTY(EditDefaultsOnly, Category = "KeyBinds")
    UInputAction* JumpAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "KeyBinds")
    UInputMappingContext* MappingContext;
    
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PawnClientRestart() override;
	
private:
    void Move(const FInputActionValue& ActionValue);
    void Look(const FInputActionValue& ActionValue);

};
