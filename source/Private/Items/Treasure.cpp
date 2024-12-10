#include "Items/Treasure.h"
#include "Interfaces/PickupInterface.h"

void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* Pickup = Cast<IPickupInterface>(OtherActor);
	if (Pickup)
	{
		Pickup->PickupTreasure(this);
		SpawnPickupSound();
		Destroy();
	}
}

