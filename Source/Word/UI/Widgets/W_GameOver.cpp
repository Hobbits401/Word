// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Widgets/W_GameOver.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UW_GameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Restart)
	{
		Button_Restart->OnClicked.RemoveAll(this);
		Button_Restart->OnClicked.AddDynamic(this, &UW_GameOver::HandleRestartClicked);
	}

	if (Button_MainMenu)
	{
		Button_MainMenu->OnClicked.RemoveAll(this);
		Button_MainMenu->OnClicked.AddDynamic(this, &UW_GameOver::HandleMainMenuClicked);
	}

	HideGameOver();
}

void UW_GameOver::ShowGameOver(int32 FinalScore, int32 BestScore, const FString& Reason)
{
	if (TextBlock_FinalScoreText)
	{
		TextBlock_FinalScoreText->SetText(FText::Format(NSLOCTEXT("WordAttack", "FinalScoreText", "{0}. Score: {1}"), FText::FromString(Reason), FinalScore));
	}

	if (TextBlock_BestScoreText)
	{
		TextBlock_BestScoreText->SetText(FText::Format(NSLOCTEXT("WordAttack", "BestScoreText", "Best: {0}"), BestScore));
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UW_GameOver::HideGameOver()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UW_GameOver::HandleRestartClicked()
{
	HideGameOver();
	OnRestartRequested.Broadcast();
}

void UW_GameOver::HandleMainMenuClicked()
{
	HideGameOver();
}
