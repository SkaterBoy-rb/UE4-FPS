// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "Kismet/KismetMathLibrary.h"
#include "../HomeworkCharacter.h"
#include "AICharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	// Set as root component
	RootComponent = CollisionComp;


	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	LifeSpan = 5.0f;
	ExploRange = 1500.0f;
	InitialLifeSpan = LifeSpan;
	Impulse = 400000.0f;
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity(), GetActorLocation());

		//Destroy();
	}
}

void AGrenade::PlayExplosion(AHomeworkCharacter* HomeWorkCharactor)
{
	GrenadeOwner = HomeWorkCharactor;
	if (CollisionComp->GetNumChildrenComponents() > 0)
	{
		USceneComponent* sphere = CollisionComp->GetChildComponent(0);
		sphere->SetVisibility(false);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, sphere, TEXT("StaticMesh"),
			FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector,
			EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None, true);
		// »÷·É
		TSet<AActor*> HitActor;
		FVector CameraLocation = GetActorLocation();
		FRotator ForwardRotator(5.0f, 0, 0);
		for (float angle = 0.0f; angle < 360.0f; angle += 1.0f)
		{
			ForwardRotator.Yaw += angle;
			FVector CameraForwardVector = UKismetMathLibrary::GetForwardVector(ForwardRotator);
			FVector EndLocation;
			TArray<AActor*> IgnoreArray;
			IgnoreArray.Add(this);
			FHitResult HitResult;
			EndLocation = CameraLocation + CameraForwardVector * ExploRange;
			bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation,
				ETraceTypeQuery::TraceTypeQuery1, false, IgnoreArray, EDrawDebugTrace::None,
				HitResult, true);
			if (HitSuccess)
			{
				if ((HitResult.Component).Get()->IsSimulatingPhysics())
				{
					if (!HitActor.Find((HitResult.Actor).Get()))
					{
						(HitResult.Component).Get()->AddImpulseAtLocation(CameraForwardVector * Impulse, GetActorLocation());
						HitActor.Add((HitResult.Actor).Get());
					}
				}
				else
				{
					AHomeworkCharacter* HWCharactor = Cast<AHomeworkCharacter>(HitResult.Actor);
					if (HWCharactor)
					{
						if (!HitActor.Find((HitResult.Actor).Get()))
						{
							HWCharactor->DamagePlayer((HitResult.Actor).Get(), this, CameraLocation, HitResult);
							HitActor.Add((HitResult.Actor).Get());
						}
					}
					else if ((HitResult.Actor).Get()->IsA(AAICharacter::StaticClass()))
					{
						AAICharacter* AICharactor = Cast<AAICharacter>(HitResult.Actor);
						if (!HitActor.Find((HitResult.Actor).Get()))
						{
							AICharactor->DamagePlayer((HitResult.Actor).Get(), this, CameraLocation, HitResult);
							HitActor.Add((HitResult.Actor).Get());
						}
					}
				}
			}
		}
	}
}

