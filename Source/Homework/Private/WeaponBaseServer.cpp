// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponBaseServer.h"
#include "../HomeworkCharacter.h"
#include "AICharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBaseServer::AWeaponBaseServer()
{
	PrimaryActorTick.bCanEverTick = true;
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	SphereCollison = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollison"));
	SphereCollison->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	SphereCollison->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollison->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	WeaponMesh->SetOwnerNoSee(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetSimulatePhysics(true);

	bReplicates = true;
}


void AWeaponBaseServer::OnOtherBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHomeworkCharacter* FPSCharacter = Cast<AHomeworkCharacter>(OtherActor);
	//UE_LOG(LogTemp, Warning, TEXT("FPSCharacter"));
	if (FPSCharacter)
	{
		EquipWeapon();
		FPSCharacter->EquipPrimary(this);
	}
	else
	{
		AAICharacter* AICharacter = Cast<AAICharacter>(OtherActor);
		if (AICharacter)
		{
			EquipWeapon(false);
			AICharacter->EquipPrimary(this);
		}

	}
}

// 被拾取的时候自己设置枪械碰撞相关属性
void AWeaponBaseServer::EquipWeapon(bool IsPlayer)
{
	if (!IsPlayer)
	{
		FString PathToLoad = "Material'/Game/Asset/Weapon/FPWeapon/Materials/M_FPGun_2.M_FPGun_2'";

		WeaponMesh->SetMaterial(0, Cast<UMaterialInterface>(StaticLoadObject(
			UMaterial::StaticClass(), nullptr, *(PathToLoad))));
	}
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetSimulatePhysics(false);

	SphereCollison->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponBaseServer::SetVisible(bool IsVisible)
{
	WeaponMesh->SetVisibility(IsVisible);
}

void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, TEXT("Fire_Slot"),
			FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector, 
			EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None, true);
	}
}

bool AWeaponBaseServer::MultiShootingEffect_Validate()
{
	return true;
}

void AWeaponBaseServer::MultiReloadEffect_Implementation()
{
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ReloadSound, GetActorLocation());
	}
}

bool AWeaponBaseServer::MultiReloadEffect_Validate()
{
	return true;
}

void AWeaponBaseServer::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AWeaponBaseServer, ClipCurrentBullet, COND_None);
	DOREPLIFETIME_CONDITION(AWeaponBaseServer, GunCurrentBullet, COND_None);
}

void AWeaponBaseServer::BeginPlay()
{
	Super::BeginPlay();
	SphereCollison->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBaseServer::OnOtherBeginOverlap);
	SetReplicates(true);
}

// Called every frame
void AWeaponBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

