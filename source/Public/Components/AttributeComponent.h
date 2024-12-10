// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float CurrentHealth;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float CurrentStamina;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StaminaRechargeRate = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Gold;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Souls;
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 DodgeStaminaCost = 10.f;

public:
	void ChangeHealth(float Amount);
	void ChangeGold(int32 Amount);
	void ChangeSouls(int32 Amount);
	void UseStamina(float Amount);
	float GetHealthPercent();
	float GetStaminaPercent();
	bool IsAlive();

	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE float GetStamina() const { return CurrentStamina; }
	FORCEINLINE float GetStaminaRechargeRate() const { return StaminaRechargeRate; }
	FORCEINLINE float GetDodgeCost() const { return DodgeStaminaCost; }
	FORCEINLINE bool CanUseDodge() const { return CurrentStamina >= DodgeStaminaCost; }
};
