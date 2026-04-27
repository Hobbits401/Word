// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_GameOver.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWARestartRequestedSignature);

UCLASS()
class WORD_API UW_GameOver : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Game Over")
	FWARestartRequestedSignature OnRestartRequested;

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game Over")
	void ShowGameOver(int32 FinalScore, int32 BestScore, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game Over")
	void HideGameOver();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_FinalScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_BestScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Restart;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MainMenu;

private:
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleMainMenuClicked();
};
