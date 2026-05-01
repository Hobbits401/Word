// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/WordTypes.h"
#include "Gameplay/WA_GameState.h"
#include "WA_PlayerController.generated.h"

class AWA_GameMode;
class UButton;
class UImage;
class UPaperSprite;
class UProgressBar;
class UTextBlock;
class UUserWidget;
class UWidgetAnimation;

UCLASS()
class WORD_API AWA_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWA_PlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Word Attack|UI")
	void HandleWordSelected(const FString& SelectedWord);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|UI")
	void HandleRestartRequested();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UUserWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UUserWidget> MainSettingsClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UUserWidget> MiniSettingsClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UUserWidget> NewScoreClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UUserWidget> PlayZoneClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TSubclassOf<UUserWidget> FailClass;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TObjectPtr<UPaperSprite> HeartAliveSprite;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|UI")
	TObjectPtr<UPaperSprite> HeartLostSprite;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainSettingsWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> MiniSettingsWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> NewScoreWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> PlayZoneWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> FailWidget;

	UPROPERTY()
	TArray<TObjectPtr<UButton>> OptionButtons;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> OptionTexts;

	UPROPERTY()
	TArray<TObjectPtr<UImage>> HeartImages;

	UPROPERTY()
	TArray<FString> OptionWords;

	bool bPendingFailAfterNewScore = false;

	UFUNCTION()
	void HandleRoundUpdated(const FWordRound& Round);

	UFUNCTION()
	void HandleScoreChanged(int32 Score);

	UFUNCTION()
	void HandleTimerUpdated(float TimeLeft);

	UFUNCTION()
	void HandleLivesChanged(int32 Lives);

	UFUNCTION()
	void HandleComboChanged(int32 Combo, float Progress);

	UFUNCTION()
	void HandleScreenChanged(EWA_GameScreen Screen, bool bIsPaused, bool bIsGameActive);

	UFUNCTION()
	void HandleGameOver(const FString& Reason, int32 FinalScore, int32 BestScore, bool bNewBestScore);

	UFUNCTION()
	void HandlePlayClicked();

	UFUNCTION()
	void HandleMainSettingsClicked();

	UFUNCTION()
	void HandleMainSettingsBackClicked();

	UFUNCTION()
	void HandlePauseClicked();

	UFUNCTION()
	void HandleMiniSettingsBackClicked();

	UFUNCTION()
	void HandleFailNewRoundClicked();

	UFUNCTION()
	void HandleFailToMenuClicked();

	UFUNCTION()
	void HandleNewScoreOkClicked();

	UFUNCTION()
	void HandleOption1Clicked();

	UFUNCTION()
	void HandleOption2Clicked();

	UFUNCTION()
	void HandleOption3Clicked();

	UFUNCTION()
	void HandleOption4Clicked();

	UFUNCTION()
	void HandleOption5Clicked();

	void CreateWidgets();
	void BindWidgetEvents();
	void BindGameStateEvents();
	void RefreshWidgetsFromState();
	void ShowOnlyMainMenu();
	void ShowMainSettings();
	void ShowPlaying();
	void ShowPause();
	void ShowGameOverWidgets(bool bNewBestScore);
	void ShowFailOnly();
	void SetWidgetVisibility(UUserWidget* Widget, bool bVisible) const;
	void UpdateBestScore(int32 BestScore);
	void UpdateOptionClicked(int32 OptionIndex);
	void CachePlayZoneControls();
	void UpdateHeartSprites(int32 Lives);
	void SetHeartSprite(UImage* Image, UPaperSprite* Sprite) const;
	void PlayWidgetAnimationByName(UUserWidget* Widget, FName AnimationName, bool bLoop);
	void StopWidgetAnimationByName(UUserWidget* Widget, FName AnimationName);
	UWidgetAnimation* FindWidgetAnimationByName(UUserWidget* Widget, FName AnimationName) const;
	void StartTimerWaitingAnimation();
	void StopTimerWaitingAnimation();
	void PlayTimerNewTimeAnimation();
	AWA_GameMode* GetWAGameMode() const;
	AWA_GameState* GetWAGameState() const;
};
