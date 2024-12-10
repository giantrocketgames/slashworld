#include "Components/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttributeComponent::BeginPlay()
{
	CurrentHealth = MaxHealth;
}

void UAttributeComponent::ChangeHealth(float Amount)
{
	CurrentHealth += Amount;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, MaxHealth);
}

void UAttributeComponent::ChangeGold(int32 Amount)
{
	Gold += Amount;
}

void UAttributeComponent::ChangeSouls(int32 Amount)
{
	Souls += Amount;
}

void UAttributeComponent::UseStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
}

float UAttributeComponent::GetHealthPercent()
{
	return CurrentHealth / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent()
{
	return CurrentStamina / MaxStamina;
}

bool UAttributeComponent::IsAlive()
{
	return CurrentHealth > 0;
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (CurrentStamina < MaxStamina)
	{
		UseStamina(-StaminaRechargeRate * DeltaTime);
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
