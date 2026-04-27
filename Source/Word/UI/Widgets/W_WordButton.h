// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_WordButton.generated.h"

class UBorder;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWAWordButtonClickedSignature, const FString&, Word);

UCLASS()
class WORD_API UW_WordButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Button")
	FWAWordButtonClickedSignature OnWordClicked;

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Button")
	void SetWord(const FString& NewWord);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Button")
	FString GetWord() const { return Word; }

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Button")
	void SetNormal();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Button")
	void SetCorrect();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Button")
	void SetIncorrect();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Button")
	void SetDisabled();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Root;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_ButtonBackground;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_WordText;

private:
	UPROPERTY()
	FString Word;

	UFUNCTION()
	void HandleClicked();

	void SetBackgroundColor(const FLinearColor& Color);
};
