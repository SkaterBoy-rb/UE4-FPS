// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class AHomeworkCharacter;

UCLASS(config = Game)
class HOMEWORK_API AGrenade : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

public:	
	AGrenade();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			FVector NormalImpulse, const FHitResult& Hit);

	float LifeSpan;

	float ExploRange;

	float Impulse;

	AHomeworkCharacter* GrenadeOwner;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;
	
	void PlayExplosion(AHomeworkCharacter* HomeWorkCharactor);


protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

};
