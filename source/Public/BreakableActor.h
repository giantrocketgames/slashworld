// Fill out your copyright notice in the Description page of Project Settings.

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

	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChaosBreakEvent, const FChaosBreakEvent&, BreakEvent);
	UFUNCTION()
	void OnChaosBreakEvent(const FChaosBreakEvent& BreakEvent);
private:	
	UPROPERTY(VisibleAnywhere)
	UGeometryCollectionComponent* GeometryCollection;

	
	bool bHit;
	UPROPERTY(EditAnywhere)
	USoundBase* BreakSound;

	UPROPERTY(EditAnywhere, Category = Breakables)
	TArray<TSubclassOf<class ATreasure>> ObjectsToSpawn;

	// Inherited via 
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
};
