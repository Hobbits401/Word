// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widgets/W_GameHUD.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/Widgets/W_WordButton.h"

void UW_GameHUD::NativeConstruct()
{
	Super::NativeConstruct();
	Init();
}

void UW_GameHUD::Init()
{
	CacheButtons();
	for (UW_WordButton* WordButton : WordButtons)
	{
		if (WordButton)
		{
			WordButton->OnWordClicked.RemoveAll(this);
			WordButton->OnWordClicked.AddDynamic(this, &UW_GameHUD::HandleWordClicked);
		}
	}

	UpdateScore(0);
	UpdateTimer(0.0f);
}

void UW_GameHUD::UpdateRound(const FWordRound& Round)
{
	CacheButtons();

	FString LettersText;
	for (const TCHAR Letter : Round.Letters)
	{
		if (!LettersText.IsEmpty())
		{
			LettersText.AppendChar(TEXT(' '));
		}
		LettersText.AppendChar(Letter);
	}

	if (TextBlock_LettersText)
	{
		TextBlock_LettersText->SetText(FText::FromString(LettersText));
	}

	for (int32 Index = 0; Index < WordButtons.Num(); ++Index)
	{
		if (WordButtons[Index] && Round.Options.IsValidIndex(Index))
		{
			WordButtons[Index]->SetWord(Round.Options[Index]);
		}
	}

	EnableButtons();
}

void UW_GameHUD::UpdateScore(int32 Score)
{
	if (TextBlock_ScoreLabel)
	{
		TextBlock_ScoreLabel->SetText(FText::Format(NSLOCTEXT("WordAttack", "ScoreLabel", "Score: {0}"), Score));
	}
}

void UW_GameHUD::UpdateTimer(float TimeLeft)
{
	if (TextBlock_TimerLabel)
	{
		TextBlock_TimerLabel->SetText(FText::Format(NSLOCTEXT("WordAttack", "TimerLabel", "{0}s"), FText::AsNumber(FMath::CeilToInt(TimeLeft))));
	}

	if (Border_TimeWarningOverlay)
	{
		Border_TimeWarningOverlay->SetVisibility(TimeLeft <= 1.0f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UW_GameHUD::DisableButtons()
{
	CacheButtons();
	for (UW_WordButton* WordButton : WordButtons)
	{
		if (WordButton)
		{
			WordButton->SetDisabled();
		}
	}
}

void UW_GameHUD::EnableButtons()
{
	CacheButtons();
	for (UW_WordButton* WordButton : WordButtons)
	{
		if (WordButton)
		{
			WordButton->SetNormal();
		}
	}
}

void UW_GameHUD::HandleWordClicked(const FString& Word)
{
	DisableButtons();
	OnWordSelected.Broadcast(Word);
}

void UW_GameHUD::CacheButtons()
{
	WordButtons = { W_WordButton_Word1, W_WordButton_Word2, W_WordButton_Word3, W_WordButton_Word4 };
}
