#include "HUD/SlashOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USlashOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetCoinsValue(int32 Coins)
{
	if (CoinsText)
	{
		const FString FormatString = FString::Printf(TEXT("%d"), Coins);
		FText FormattedText = FText::FromString(FormatString);
		CoinsText->SetText(FormattedText);
	}
}

void USlashOverlay::SetSoulsValue(int32 Souls)
{
	if (SoulsText)
	{
		const FString FormatString = FString::Printf(TEXT("%d"), Souls);
		FText FormattedText = FText::FromString(FormatString);
		SoulsText->SetText(FormattedText);
	}
}
