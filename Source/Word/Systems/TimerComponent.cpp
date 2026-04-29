// Copyright Epic Games, Inc. All Rights Reserved.

#include "Systems/TimerComponent.h"

UTimerComponent::UTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTimerComponent::StartTimer(float InitialTime)
{
	TimeLeft = FMath::Max(0.0f, InitialTime);
	bIsRunning = true;
	OnTimerUpdated.Broadcast(TimeLeft);
}

void UTimerComponent::ResumeTimer()
{
	bIsRunning = TimeLeft > 0.0f;
	OnTimerUpdated.Broadcast(TimeLeft);
}

void UTimerComponent::StopTimer()
{
	bIsRunning = false;
}

void UTimerComponent::AddTime(float TimeToAdd)
{
	TimeLeft = FMath::Max(0.0f, TimeLeft + TimeToAdd);
	OnTimerUpdated.Broadcast(TimeLeft);
}

void UTimerComponent::SetDrainMultiplier(float NewDrainMultiplier)
{
	DrainMultiplier = FMath::Max(0.0f, NewDrainMultiplier);
}

void UTimerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickTimer(DeltaTime);
}

void UTimerComponent::TickTimer(float DeltaTime)
{
	if (!bIsRunning)
	{
		return;
	}

	TimeLeft -= DeltaTime * DrainMultiplier;
	if (TimeLeft <= 0.0f)
	{
		TimeLeft = 0.0f;
		bIsRunning = false;
		OnTimerUpdated.Broadcast(TimeLeft);
		OnTimerExpired.Broadcast();
		return;
	}

	OnTimerUpdated.Broadcast(TimeLeft);
}
