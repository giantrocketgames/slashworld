// Fill out your copyright notice in the Description page of Project Settings.

#include "Interfaces/PickupInterface.h"
#include "Items/Soul.h"

void ASoul::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* Pickup = Cast<IPickupInterface>(OtherActor);
	if (Pickup)
	{
		Pickup->PickupSoul(this);
		SpawnPickupSystem();
		SpawnPickupSound();
		Destroy();
	}
}

