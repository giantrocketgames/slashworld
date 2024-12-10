#pragma once

#include "CoreMinimal.h"
#include "Interfaces/HitInterface.h"
#include "GameFramework/Actor.h"
#include "BreakableActor.generated.h"

class UGeometryCollectionComponent;
class USoundBase;
class UCapsuleComponent;

UCLASS()
class SLASH_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCapsuleComponent* Capsule;

	UFUNCTION()
	void OnChaosBreakEvent(const FChaosBreakEvent& BreakEvent);

private:	
	UPROPERTY(VisibleAnywhere)
	UGeometryCollectionComponent* GeometryCollection;
	
	UPROPERTY(EditAnywhere)
	USoundBase* BreakSound;

	UPROPERTY(EditAnywhere, Category = Breakables)
	TArray<TSubclassOf<class ATreasure>> ObjectsToSpawn;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	bool bHit;
};
