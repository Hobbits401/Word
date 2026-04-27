// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/WordTypes.h"
#include "WA_PlayerController.generated.h"

class AWA_GameMode;
class AWA_GameState;
class UW_GameHUD;
class UW_GameOver;

UCLASS()
class WORD_API AWA_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Word Attack|UI")
	void HandleWordSelected(const FString& SelectedWord);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|UI")
	void HandleRestartRequested();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UW_GameHUD> GameHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UW_GameOver> GameOverClass;

private:
	UPROPERTY()
	TObjectPtr<UW_GameHUD> GameHUD;

	UPROPERTY()
	TObjectPtr<UW_GameOver> GameOverWidget;

	UFUNCTION()
	void HandleRoundUpdated(const FWordRound& Round);

	UFUNCTION()
	void HandleScoreChanged(int32 Score);

	UFUNCTION()
	void HandleTimerUpdated(float TimeLeft);

	UFUNCTION()
	void HandleGameOver(const FString& Reason, int32 FinalScore, int32 BestScore);

	void CreateWidgets();
	void BindGameStateEvents();
	void RefreshWidgetsFromState();
	AWA_GameMode* GetWAGameMode() const;
	AWA_GameState* GetWAGameState() const;
};
