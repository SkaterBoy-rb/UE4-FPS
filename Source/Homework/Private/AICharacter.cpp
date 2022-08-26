// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "../HomeworkCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/CapsuleComponent.h"
#include "AICharacterController.h"
#include "GameFramework/CharacterMovementComponent.h"

const TMap<EWeaponType, FName> BodyLocation = {
	{EWeaponType::FPS, TEXT("Weapon_FPS")},
	{EWeaponType::Sniper, TEXT("Weapon_Sniper")}
};
// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
}

AWeaponBaseServer* AAICharacter::GetCurrentServerWeapon()
{
	return ServerPrimaryWeapon;
}

void AAICharacter::EquipPrimary(AWeaponBaseServer* WeaponBaseServer)
{
	if (!ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		ServerPrimaryWeapon->K2_AttachToComponent(GetMesh(), BodyLocation[ActiveWeapon],
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, 
			EAttachmentRule::SnapToTarget, true);
	}
}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	HP = 100;
	ActiveWeapon = FMath::RandRange(0, 1) == 0 ? EWeaponType::FPS : EWeaponType::Sniper;
	OnTakePointDamage.AddDynamic(this, &AAICharacter::OnHit);
	ServerBodysAnimBP = GetMesh()->GetAnimInstance();

	AIControllerClass = AAICharacterController::StaticClass();
	StartWithKindofWeapon();
}

void AAICharacter::StartWithKindofWeapon()
{
	if (HasAuthority())
	{
		PurchaseWeapon(ActiveWeapon);
	}
}

void AAICharacter::PurchaseWeapon(EWeaponType WeaponType)
{
	FActorSpawnParameters SapwnInfo;
	SapwnInfo.Owner = this;
	SapwnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	switch (WeaponType)
	{
	case EWeaponType::FPS:
	{
		UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,
			TEXT("BluePrint'/Game/BluePrint/weapon/FPS/ServerBP_FPS.ServerBP_FPS_C'"));
		AWeaponBaseServer* ServerWeapon =
			GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SapwnInfo);
		ServerWeapon->EquipWeapon(false);
		EquipPrimary(ServerWeapon);
		break;
	}
	case EWeaponType::Sniper:
	{
		UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,
			TEXT("BluePrint'/Game/BluePrint/weapon/Sniper/ServerBP_Sniper.ServerBP_Sniper_C'"));
		AWeaponBaseServer* ServerWeapon =
			GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar, GetActorTransform(), SapwnInfo);
		ServerWeapon->EquipWeapon();
		EquipPrimary(ServerWeapon);
		break;
	}
	default:
		break;
	}
}

void AAICharacter::DamagePlayer(AActor* DamageActor, AActor* DamageCauser, FVector& HitFromDirection, FHitResult& HitInfo)
{
	AAICharacter* AICharacter = Cast<AAICharacter>(DamageCauser);
	float Damage;
	if (AICharacter)
	{
		Damage = AICharacter->GetCurrentServerWeapon()->BaseDamage;
		UPhysicalMaterial* PhysicalMaterial = (HitInfo.PhysMaterial).Get();
		Damage = ServerPrimaryWeapon->BaseDamage;
		switch (PhysicalMaterial->SurfaceType)
		{
		case EPhysicalSurface::SurfaceType1:
		{
			// head
			Damage *= 4;
			break;
		}
		case EPhysicalSurface::SurfaceType2:
		{
			// body
			Damage *= 1;
			break;
		}
		case EPhysicalSurface::SurfaceType3:
		{
			// arm
			Damage *= 0.8;
			break;
		}
		case EPhysicalSurface::SurfaceType4:
		{
			// leg
			Damage *= 0.7;
			break;
		}
		}
	}
	else
	{
		// 投掷物（grenade）
		float Distance = pow(DamageCauser->GetActorLocation().X - DamageActor->GetActorLocation().X, 2)
			+ pow(DamageCauser->GetActorLocation().Y - DamageActor->GetActorLocation().Y, 2);
		if (Distance < 160000)
			Damage = 100;
		else if (Distance < 1000000)
			Damage = 50;
		else
			Damage = 20;
	}
	// 底层观察者模式,打了别人发通知
	UGameplayStatics::ApplyPointDamage(DamageActor, Damage, HitFromDirection,
		HitInfo, GetController(), DamageCauser, UDamageType::StaticClass());
	// 当自己被打时的回调OnHit
}

void AAICharacter::Dead(AActor* DamageCauser)
{
	if (DamageCauser)
	{
		DeathMatch(DamageCauser);
	}
	MultiDead();
}

void AAICharacter::DelayPlayDeadCallBack()
{
	ServerPrimaryWeapon->Destroy();
	Destroy();
}

void AAICharacter::OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	if (HP == 0)
		return;
	HP = (HP > Damage) ? HP - Damage : 0;
	AGrenade* DamageSrc = Cast<AGrenade>(DamageCauser);
	if (HP == 0)
	{
		// 死亡逻辑
		if (DamageSrc)
			DamageCauser = DamageSrc->GrenadeOwner;
		Dead(DamageCauser);
	}
	else
	{
		if (DamageSrc)
			Dead(nullptr);
	}
}

void AAICharacter::FireWeaponPrimary()
{
	MultiShooting();
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAICharacter::MultiShooting_Implementation()
{
	if (ServerPrimaryWeapon)
	{
		// 多播射击特效
		ServerPrimaryWeapon->MultiShootingEffect();
		// 多播身体动画
		if (ServerBodysAnimBP)
		{
			ServerBodysAnimBP->Montage_Play(ServerPrimaryWeapon->ServerTPBodysShootAnimMontage);
		}

		//RifleLineTrace(CameraLocation, CameraRotation, IsMoving);

	}
}

bool AAICharacter::MultiShooting_Validate()
{
	return true;
}

void AAICharacter::MultiDead_Implementation()
{
	if (ServerBodysAnimBP)
	{
		ServerBodysAnimBP->Montage_Play(ServerTPBodysDeadAnimMontage);
	}
	if (HP == 0)
	{
		FLatentActionInfo ActionInfo(0, FMath::Rand(), TEXT("DelayPlayDeadCallBack"), this);
		UKismetSystemLibrary::Delay(this,
			ServerTPBodysDeadAnimMontage->GetPlayLength() / 2, ActionInfo);
	}
}

bool AAICharacter::MultiDead_Validate()
{
	return true;
}

