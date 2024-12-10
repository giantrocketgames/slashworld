#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Soul.generated.h"

UCLASS()
class SLASH_API ASoul : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;


private:
	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 Value;

public:
	FORCEINLINE int32 GetSoulValue() const { return Value; }
};
