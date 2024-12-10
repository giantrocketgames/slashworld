// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{
	BoxTraceStart = CreateDefaultSubobject<USceneComponent>("BoxTraceStart");
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>("BoxTraceEnd");
	BoxTraceEnd->SetupAttachment(GetRootComponent());
	WeaponBox = CreateDefaultSubobject<UBoxComponent>("WeaponBox");
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnWeaponOverlap);
	WeaponBox->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnWeaponOverlapEnd);
}

void AWeapon::Equip(USceneComponent* InParent, FName SocketName, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;

	SetOwner(NewOwner);
	// Actor can be owned by any Pawn, which is instigator
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, SocketName);
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}

	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (DisplayNiagaraComponent)
	{
		DisplayNiagaraComponent->Deactivate();
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& SocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, SocketName);
}

void AWeapon::SetWeaponCollisionEnable(ECollisionEnabled::Type CollisionEnabled)
{
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(CollisionEnabled);
	}

	if (CollisionEnabled == ECollisionEnabled::NoCollision)
	{
		ActorsHitThisAttack.Empty();
	}
}

void AWeapon::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy")))
	{
		return;
	}

	FHitResult BoxHit;
	BoxTrace(BoxHit);
	AActor* HitActor = BoxHit.GetActor();
	if (HitActor)
	{
		ActorsHitThisAttack.AddUnique(HitActor);
		if (GetOwner()->ActorHasTag(TEXT("Enemy")) && HitActor->ActorHasTag(TEXT("Enemy")))
		{
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitActor->GetName());

		// Hit them (play hit reacts etc) FIRST
		ExecuteOnIHitInterface(HitActor, BoxHit);


		// After they are hit then Apply Damage
		UGameplayStatics::ApplyDamage(
			HitActor,
			Damage,
			GetInstigator()->Controller,
			this,
			UDamageType::StaticClass());
	}
}

void AWeapon::ExecuteOnIHitInterface(AActor* HitActor, FHitResult& BoxHit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(HitActor);
	if (HitInterface)
	{
		//HitInterface->GetHit(HitResult.ImpactPoint);
		HitInterface->Execute_GetHit(HitActor, BoxHit.ImpactPoint, Owner);
		CreateFields(BoxHit.ImpactPoint);
	}
}

void AWeapon::OnWeaponOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();
	TArray <AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(Owner);

	for (AActor* Actor : ActorsHitThisAttack)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Need to ignore actor: %s"), *Actor->GetName());
		ActorsToIgnore.AddUnique(Actor);
	}
	//UE_LOG(LogTemp, Warning, TEXT("Ignoring: %i Actors"), ActorsToIgnore.Num());
	bool bHit = UKismetSystemLibrary::BoxTraceSingle(this, Start, End,
		BoxTraceExtents,
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		//EDrawDebugTrace::Type::ForDuration,
		bShowBoxDebug ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None,
		BoxHit,
		true);
}
