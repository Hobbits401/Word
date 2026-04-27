// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YandexSDKManager.generated.h"

UCLASS(BlueprintType)
class WORD_API UYandexSDKManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Word Attack|Yandex")
	void InitializeSDK() {}
};
