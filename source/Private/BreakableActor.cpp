#include "BreakableActor.h"
#include "Items/Treasure.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);


	SetRootComponent(GeometryCollection);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	Capsule->SetGenerateOverlapEvents(false);

}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnChaosBreakEvent);
}

void ABreakableActor::OnChaosBreakEvent(const FChaosBreakEvent& BreakEvent)
{
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (bHit)
	{
		return;
	}

	bHit = true;

	if (BreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BreakSound,
			ImpactPoint
		);
	}

	UWorld* World = GetWorld();
	if (World && ObjectsToSpawn.Num() > 0)
	{
		int32 max = ObjectsToSpawn.Num();
		int32 idx = FMath::RandRange(0, max - 1);
		World->SpawnActor<ATreasure>(ObjectsToSpawn[idx], GetActorLocation() + FVector::UpVector * 50.f, GetActorRotation());
	}
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

}

