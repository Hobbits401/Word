// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/WordTypes.h"
#include "W_GameHUD.generated.h"

class UBorder;
class UTextBlock;
class UVerticalBox;
class UW_WordButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWAWordSelectedSignature, const FString&, Word);

UCLASS()
class WORD_API UW_GameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Word Attack|HUD")
	FWAWordSelectedSignature OnWordSelected;

	UFUNCTION(BlueprintCallable, Category = "Word Attack|HUD")
	void Init();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|HUD")
	void UpdateRound(const FWordRound& Round);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|HUD")
	void UpdateScore(int32 Score);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|HUD")
	void UpdateTimer(float TimeLeft);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|HUD")
	void DisableButtons();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|HUD")
	void EnableButtons();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_ScoreLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_TimerLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_LettersPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_LettersText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_WordsContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UW_WordButton> W_WordButton_Word1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UW_WordButton> W_WordButton_Word2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UW_WordButton> W_WordButton_Word3;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UW_WordButton> W_WordButton_Word4;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_TimeWarningOverlay;

private:
	UPROPERTY()
	TArray<TObjectPtr<UW_WordButton>> WordButtons;

	UFUNCTION()
	void HandleWordClicked(const FString& Word);

	void CacheButtons();
};
