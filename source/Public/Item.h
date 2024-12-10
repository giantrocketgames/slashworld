#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;

enum class EItemState : uint8
{
	EIS_Hovering,
	EIS_Equipped
};

UCLASS()
class SLASH_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();
	virtual void Tick(float DeltaTime) override;
	UStaticMeshComponent* GetMesh();

protected:
	virtual void BeginPlay() override;
	virtual void SpawnPickupSystem();
	virtual void SpawnPickupSound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sin Parameters")
		float _amplitude = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sin Parameters")
		float _period = 5.f;
	UFUNCTION(BlueprintPure)
		float TransformedSin();
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh;

	UFUNCTION()
		virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	EItemState ItemState = EItemState::EIS_Hovering;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UNiagaraComponent* DisplayNiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* PickupEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundBase* PickupSound;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float _lifeTime = 0.f;

};