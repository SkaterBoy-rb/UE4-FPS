// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacterController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

void AAICharacterController::OnPossess(class APawn* InPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("OnPossess"));
	Super::OnPossess(InPawn);
	AICharacter = Cast<AAICharacter>(InPawn);
	SearchNewPoint();
}

void AAICharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	SearchNewPoint();
}

void AAICharacterController::SearchNewPoint()
{
	UNavigationSystemV1* NavMesh = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	if (NavMesh)
	{
		const float SearchRadius = 5000.0f;
		FNavLocation RandomPt;
		const bool bIsFound = NavMesh->GetRandomReachablePointInRadius(AICharacter->GetActorLocation(), SearchRadius, RandomPt);
		if (bIsFound)
		{
			MoveToLocation(RandomPt);
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("NavMesh NULL"));
}
