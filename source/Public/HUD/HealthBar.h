// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"


UCLASS()
class SLASH_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
public:
	// Will link up based on name. Needs to match what's in the blueprint
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
};
