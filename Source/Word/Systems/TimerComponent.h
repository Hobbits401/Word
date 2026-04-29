// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWATimerUpdatedSignature, float, TimeLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWATimerExpiredSignature);

UCLASS(ClassGroup=(WordAttack), meta=(BlueprintSpawnableComponent))
class WORD_API UTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTimerComponent();

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Timer")
	FWATimerUpdatedSignature OnTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Timer")
	FWATimerExpiredSignature OnTimerExpired;

	void StartTimer(float InitialTime);
	void ResumeTimer();
	void StopTimer();
	void AddTime(float TimeToAdd);
	void SetDrainMultiplier(float NewDrainMultiplier);
	float GetTimeLeft() const { return TimeLeft; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bIsRunning = false;
	float TimeLeft = 0.0f;
	float DrainMultiplier = 1.0f;

	void TickTimer(float DeltaTime);
};
