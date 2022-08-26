// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICharacter.h"
#include "AICharacterController.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK_API AAICharacterController : public AAIController
{
	GENERATED_BODY()

public:
	void OnPossess(class APawn* InPawn) override;
	void Tick(float DeltaTime) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void SearchNewPoint();

private:
	class AAICharacter* AICharacter;
};
