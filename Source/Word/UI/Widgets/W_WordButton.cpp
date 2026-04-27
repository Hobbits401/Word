// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widgets/W_WordButton.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UW_WordButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Root)
	{
		Button_Root->OnClicked.RemoveAll(this);
		Button_Root->OnClicked.AddDynamic(this, &UW_WordButton::HandleClicked);
	}

	SetNormal();
}

void UW_WordButton::SetWord(const FString& NewWord)
{
	Word = NewWord;
	if (TextBlock_WordText)
	{
		TextBlock_WordText->SetText(FText::FromString(Word));
	}
	SetNormal();
}

void UW_WordButton::SetNormal()
{
	SetIsEnabled(true);
	if (Button_Root)
	{
		Button_Root->SetIsEnabled(true);
	}
	SetBackgroundColor(FLinearColor(0.08f, 0.10f, 0.14f, 1.0f));
}

void UW_WordButton::SetCorrect()
{
	SetBackgroundColor(FLinearColor(0.05f, 0.45f, 0.20f, 1.0f));
}

void UW_WordButton::SetIncorrect()
{
	SetBackgroundColor(FLinearColor(0.65f, 0.08f, 0.08f, 1.0f));
}

void UW_WordButton::SetDisabled()
{
	SetIsEnabled(false);
	if (Button_Root)
	{
		Button_Root->SetIsEnabled(false);
	}
	SetBackgroundColor(FLinearColor(0.18f, 0.18f, 0.20f, 1.0f));
}

void UW_WordButton::HandleClicked()
{
	OnWordClicked.Broadcast(Word);
}

void UW_WordButton::SetBackgroundColor(const FLinearColor& Color)
{
	if (Border_ButtonBackground)
	{
		Border_ButtonBackground->SetBrushColor(Color);
	}
}
