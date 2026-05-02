// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/WA_PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ContentWidget.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Gameplay/WA_GameMode.h"
#include "MovieScene.h"
#include "PaperSprite.h"
#include "Slate/SlateTextureAtlasInterface.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogWAPlayerController, Log, All);

namespace
{
template <typename WidgetType>
WidgetType* FindNamedWidget(UUserWidget* Root, const FName WidgetName)
{
	return Root && Root->WidgetTree ? Cast<WidgetType>(Root->WidgetTree->FindWidget(WidgetName)) : nullptr;
}

void SetText(UUserWidget* Root, const FName WidgetName, const FText& Text)
{
	if (UTextBlock* TextBlock = FindNamedWidget<UTextBlock>(Root, WidgetName))
	{
		TextBlock->SetText(Text);
	}
}

void BindButton(UUserWidget* Root, const FName WidgetName, UObject* Target, FName FunctionName)
{
	if (UButton* Button = FindNamedWidget<UButton>(Root, WidgetName))
	{
		Button->OnClicked.Clear();
		FScriptDelegate Delegate;
		Delegate.BindUFunction(Target, FunctionName);
		Button->OnClicked.Add(Delegate);
	}
}

void CollectDescendants(UWidget* Root, TArray<UWidget*>& OutWidgets)
{
	if (!Root)
	{
		return;
	}

	OutWidgets.Add(Root);

	if (UPanelWidget* Panel = Cast<UPanelWidget>(Root))
	{
		for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
		{
			CollectDescendants(Panel->GetChildAt(Index), OutWidgets);
		}
		return;
	}

	if (UContentWidget* Content = Cast<UContentWidget>(Root))
	{
		CollectDescendants(Content->GetContent(), OutWidgets);
	}
}

template <typename WidgetType>
WidgetType* FindFirstDescendantOfClass(UWidget* Root)
{
	TArray<UWidget*> Widgets;
	CollectDescendants(Root, Widgets);
	for (UWidget* Widget : Widgets)
	{
		if (WidgetType* TypedWidget = Cast<WidgetType>(Widget))
		{
			return TypedWidget;
		}
	}

	return nullptr;
}

FText FormatTimer(float TimeLeft)
{
	const int32 TotalSeconds = FMath::Max(0, FMath::CeilToInt(TimeLeft));
	return FText::Format(NSLOCTEXT("WordAttack", "TimerFormat", "{0}:{1}"),
		FText::AsNumber(TotalSeconds / 60),
		FText::AsNumber(TotalSeconds % 60));
}
}

AWA_PlayerController::AWA_PlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuFinder(TEXT("/Game/UI/WBP_MainMenue"));
	if (MainMenuFinder.Succeeded())
	{
		MainMenuClass = MainMenuFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> MainSettingsFinder(TEXT("/Game/UI/WBP_MainSettings"));
	if (MainSettingsFinder.Succeeded())
	{
		MainSettingsClass = MainSettingsFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> MiniSettingsFinder(TEXT("/Game/UI/WBP_MiniSettings"));
	if (MiniSettingsFinder.Succeeded())
	{
		MiniSettingsClass = MiniSettingsFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> NewScoreFinder(TEXT("/Game/UI/WBP_NewScore"));
	if (NewScoreFinder.Succeeded())
	{
		NewScoreClass = NewScoreFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> PlayZoneFinder(TEXT("/Game/UI/WBP_PlayZone"));
	if (PlayZoneFinder.Succeeded())
	{
		PlayZoneClass = PlayZoneFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> FailFinder(TEXT("/Game/UI/WBP_Fail"));
	if (FailFinder.Succeeded())
	{
		FailClass = FailFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UPaperSprite> HeartAliveSpriteFinder(TEXT("/Game/UI/UI_Elements/Heart/Heart_FLipTexture_Sprite_0.Heart_FLipTexture_Sprite_0"));
	if (HeartAliveSpriteFinder.Succeeded())
	{
		HeartAliveSprite = HeartAliveSpriteFinder.Object;
	}
	UE_LOG(LogWAPlayerController, Log, TEXT("HeartAliveSprite load: %s"), HeartAliveSprite ? *HeartAliveSprite->GetPathName() : TEXT("FAILED"));

	static ConstructorHelpers::FObjectFinder<UPaperSprite> HeartLostSpriteFinder(TEXT("/Game/UI/UI_Elements/Heart/Heart_FLipTexture_Sprite_18.Heart_FLipTexture_Sprite_18"));
	if (HeartLostSpriteFinder.Succeeded())
	{
		HeartLostSprite = HeartLostSpriteFinder.Object;
	}
	UE_LOG(LogWAPlayerController, Log, TEXT("HeartLostSprite load: %s"), HeartLostSprite ? *HeartLostSprite->GetPathName() : TEXT("FAILED"));

	HeartLossSprites.Reset();
	for (int32 FrameIndex = 0; FrameIndex <= 18; ++FrameIndex)
	{
		const FString SpritePath = FString::Printf(TEXT("/Game/UI/UI_Elements/Heart/Heart_FLipTexture_Sprite_%d.Heart_FLipTexture_Sprite_%d"), FrameIndex, FrameIndex);
		UPaperSprite* FrameSprite = LoadObject<UPaperSprite>(nullptr, *SpritePath);
		HeartLossSprites.Add(FrameSprite);
		UE_LOG(LogWAPlayerController, Log, TEXT("HeartLossSprites[%d] load: %s"), FrameIndex, FrameSprite ? *FrameSprite->GetPathName() : TEXT("FAILED"));
	}

	BackgroundTextures.Reset();
	for (int32 BackgroundIndex = 1; BackgroundIndex <= 7; ++BackgroundIndex)
	{
		const FString TexturePath = FString::Printf(TEXT("/Game/UI/UI_Elements/BackGround/BackGround_%d.BackGround_%d"), BackgroundIndex, BackgroundIndex);
		UTexture2D* BackgroundTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
		BackgroundTextures.Add(BackgroundTexture);
		UE_LOG(LogWAPlayerController, Log, TEXT("BackgroundTextures[%d] load: %s"), BackgroundIndex - 1, BackgroundTexture ? *BackgroundTexture->GetPathName() : TEXT("FAILED"));
	}

	AlreadyNightSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/Sounds/Teacher/T_AlreadyNight_3.T_AlreadyNight_3"));
	UE_LOG(LogWAPlayerController, Log, TEXT("AlreadyNightSound load: %s"), AlreadyNightSound ? *AlreadyNightSound->GetPathName() : TEXT("FAILED"));
}

void AWA_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	CreateWidgets();
	BindWidgetEvents();
	BindGameStateEvents();
	RefreshWidgetsFromState();
}

void AWA_PlayerController::HandleWordSelected(const FString& SelectedWord)
{
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->HandleAnswer(SelectedWord);
	}
}

void AWA_PlayerController::HandleRestartRequested()
{
	ResetPlayZoneBackground();
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->RestartGame();
	}
}

void AWA_PlayerController::HandleRoundUpdated(const FWordRound& Round)
{
	PlayTimerNewTimeAnimation();
	StartTimerWaitingAnimation();

	SetText(PlayZoneWidget, TEXT("Text_Letter_1"), Round.Letters.IsValidIndex(0) ? FText::FromString(FString::Chr(Round.Letters[0])) : FText::GetEmpty());
	SetText(PlayZoneWidget, TEXT("Text_Letter_2"), Round.Letters.IsValidIndex(1) ? FText::FromString(FString::Chr(Round.Letters[1])) : FText::GetEmpty());
	SetText(PlayZoneWidget, TEXT("Text_Letter_3"), Round.Letters.IsValidIndex(2) ? FText::FromString(FString::Chr(Round.Letters[2])) : FText::GetEmpty());

	OptionWords = Round.Options;
	for (int32 Index = 0; Index < OptionTexts.Num(); ++Index)
	{
		const bool bHasOption = Round.Options.IsValidIndex(Index);
		if (OptionTexts[Index])
		{
			OptionTexts[Index]->SetText(bHasOption ? FText::FromString(Round.Options[Index]) : FText::GetEmpty());
		}
		if (OptionButtons.IsValidIndex(Index) && OptionButtons[Index])
		{
			OptionButtons[Index]->SetVisibility(bHasOption ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			OptionButtons[Index]->SetIsEnabled(bHasOption);
		}
	}
}

void AWA_PlayerController::HandleScoreChanged(int32 Score)
{
	SetText(PlayZoneWidget, TEXT("Text_ScoreCount"), FText::AsNumber(Score));
}

void AWA_PlayerController::HandleTimerUpdated(float TimeLeft)
{
	SetText(PlayZoneWidget, TEXT("TextTimeCount"), FormatTimer(TimeLeft));

	const AWA_GameState* WAGameState = GetWAGameState();
	const float Percent = WAGameState ? FMath::Clamp(1.0f - TimeLeft / WAGameState->RoundTimeLimit, 0.0f, 1.0f) : 0.0f;
	if (UProgressBar* ProgressBar = FindNamedWidget<UProgressBar>(PlayZoneWidget, TEXT("ProgressBar_Timer")))
	{
		ProgressBar->SetPercent(Percent);
	}
}

void AWA_PlayerController::HandleLivesChanged(int32 Lives)
{
	UE_LOG(LogWAPlayerController, Log, TEXT("HandleLivesChanged: Lives=%d LastDisplayedLives=%d HeartImages=%d"), Lives, LastDisplayedLives, HeartImages.Num());
	UpdateHeartSprites(Lives);
}

void AWA_PlayerController::HandleComboChanged(int32 Combo, float Progress)
{
	SetText(PlayZoneWidget, TEXT("Text_Combo"), FText::Format(NSLOCTEXT("WordAttack", "ComboFormat", "Комбо x {0}"), Combo));
	UpdateBackgroundByCombo(Combo);
	if (UProgressBar* ProgressBar = FindNamedWidget<UProgressBar>(PlayZoneWidget, TEXT("ProgressBar_Combo")))
	{
		ProgressBar->SetPercent(Progress);
	}
}

void AWA_PlayerController::HandleScreenChanged(EWA_GameScreen Screen, bool bIsPaused, bool bIsGameActive)
{
	switch (Screen)
	{
	case EWA_GameScreen::MainMenu:
		StopTimerWaitingAnimation();
		ShowOnlyMainMenu();
		break;
	case EWA_GameScreen::Playing:
		ShowPlaying();
		StartTimerWaitingAnimation();
		break;
	case EWA_GameScreen::Paused:
		StopTimerWaitingAnimation();
		ShowPause();
		break;
	case EWA_GameScreen::GameOver:
		StopTimerWaitingAnimation();
		SetWidgetVisibility(PlayZoneWidget, false);
		break;
	default:
		break;
	}
}

void AWA_PlayerController::HandleGameOver(const FString& Reason, int32 FinalScore, int32 BestScore, bool bNewBestScore)
{
	SetText(FailWidget, TEXT("Text_RoundScore_Numder"), FText::AsNumber(FinalScore));
	const int32 NewScoreValue = bNewBestScore ? FinalScore : BestScore;
	SetText(NewScoreWidget, TEXT("Text_NewScore_Number"), FText::AsNumber(NewScoreValue));
	SetText(NewScoreWidget, TEXT("Text_NewScore_Numder"), FText::AsNumber(NewScoreValue));
	UpdateBestScore(BestScore);
	ShowGameOverWidgets(bNewBestScore);
}

void AWA_PlayerController::HandlePlayClicked()
{
	ResetPlayZoneBackground();
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->StartGame();
	}
}

void AWA_PlayerController::HandleMainSettingsClicked()
{
	ShowMainSettings();
}

void AWA_PlayerController::HandleMainSettingsBackClicked()
{
	ShowOnlyMainMenu();
}

void AWA_PlayerController::HandlePauseClicked()
{
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->PauseGame();
	}
}

void AWA_PlayerController::HandleMiniSettingsBackClicked()
{
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->ResumeGame();
	}
}

void AWA_PlayerController::HandleFailNewRoundClicked()
{
	HandleRestartRequested();
}

void AWA_PlayerController::HandleFailToMenuClicked()
{
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->ReturnToMainMenu();
	}
}

void AWA_PlayerController::HandleNewScoreOkClicked()
{
	SetWidgetVisibility(NewScoreWidget, false);
	if (bPendingFailAfterNewScore)
	{
		bPendingFailAfterNewScore = false;
		ShowFailOnly();
	}
}

void AWA_PlayerController::HandleOption1Clicked()
{
	UpdateOptionClicked(0);
}

void AWA_PlayerController::HandleOption2Clicked()
{
	UpdateOptionClicked(1);
}

void AWA_PlayerController::HandleOption3Clicked()
{
	UpdateOptionClicked(2);
}

void AWA_PlayerController::HandleOption4Clicked()
{
	UpdateOptionClicked(3);
}

void AWA_PlayerController::HandleOption5Clicked()
{
	UpdateOptionClicked(4);
}

void AWA_PlayerController::CreateWidgets()
{
	if (MainMenuClass)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuClass);
		MainMenuWidget->AddToViewport(0);
	}
	if (PlayZoneClass)
	{
		PlayZoneWidget = CreateWidget<UUserWidget>(this, PlayZoneClass);
		PlayZoneWidget->AddToViewport(1);
	}
	if (MainSettingsClass)
	{
		MainSettingsWidget = CreateWidget<UUserWidget>(this, MainSettingsClass);
		MainSettingsWidget->AddToViewport(5);
	}
	if (MiniSettingsClass)
	{
		MiniSettingsWidget = CreateWidget<UUserWidget>(this, MiniSettingsClass);
		MiniSettingsWidget->AddToViewport(6);
	}
	if (FailClass)
	{
		FailWidget = CreateWidget<UUserWidget>(this, FailClass);
		FailWidget->AddToViewport(10);
	}
	if (NewScoreClass)
	{
		NewScoreWidget = CreateWidget<UUserWidget>(this, NewScoreClass);
		NewScoreWidget->AddToViewport(11);
	}

	CachePlayZoneControls();
	ShowOnlyMainMenu();
}

void AWA_PlayerController::BindWidgetEvents()
{
	BindButton(MainMenuWidget, TEXT("Button_Play"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandlePlayClicked));
	BindButton(MainMenuWidget, TEXT("Button_Settings"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleMainSettingsClicked));
	BindButton(MainSettingsWidget, TEXT("Button_Back"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleMainSettingsBackClicked));
	BindButton(MiniSettingsWidget, TEXT("Button_Back"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleMiniSettingsBackClicked));
	BindButton(PlayZoneWidget, TEXT("Button_Pause"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandlePauseClicked));
	BindButton(FailWidget, TEXT("Button_NewRound"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleFailNewRoundClicked));
	BindButton(FailWidget, TEXT("Button_ToMenue"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleFailToMenuClicked));
	BindButton(NewScoreWidget, TEXT("Button_Ok"), this, GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleNewScoreOkClicked));

	const FName Handlers[] = {
		GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleOption1Clicked),
		GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleOption2Clicked),
		GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleOption3Clicked),
		GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleOption4Clicked),
		GET_FUNCTION_NAME_CHECKED(AWA_PlayerController, HandleOption5Clicked)
	};

	for (int32 Index = 0; Index < OptionButtons.Num() && Index < UE_ARRAY_COUNT(Handlers); ++Index)
	{
		if (OptionButtons[Index])
		{
			OptionButtons[Index]->OnClicked.Clear();
			FScriptDelegate Delegate;
			Delegate.BindUFunction(this, Handlers[Index]);
			OptionButtons[Index]->OnClicked.Add(Delegate);
		}
	}
}

void AWA_PlayerController::BindGameStateEvents()
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState)
	{
		return;
	}

	WAGameState->OnRoundUpdated.RemoveAll(this);
	WAGameState->OnRoundUpdated.AddDynamic(this, &AWA_PlayerController::HandleRoundUpdated);
	WAGameState->OnScoreChanged.RemoveAll(this);
	WAGameState->OnScoreChanged.AddDynamic(this, &AWA_PlayerController::HandleScoreChanged);
	WAGameState->OnTimerUpdated.RemoveAll(this);
	WAGameState->OnTimerUpdated.AddDynamic(this, &AWA_PlayerController::HandleTimerUpdated);
	WAGameState->OnLivesChanged.RemoveAll(this);
	WAGameState->OnLivesChanged.AddDynamic(this, &AWA_PlayerController::HandleLivesChanged);
	WAGameState->OnComboChanged.RemoveAll(this);
	WAGameState->OnComboChanged.AddDynamic(this, &AWA_PlayerController::HandleComboChanged);
	WAGameState->OnScreenChanged.RemoveAll(this);
	WAGameState->OnScreenChanged.AddDynamic(this, &AWA_PlayerController::HandleScreenChanged);
	WAGameState->OnGameOver.RemoveAll(this);
	WAGameState->OnGameOver.AddDynamic(this, &AWA_PlayerController::HandleGameOver);
}

void AWA_PlayerController::RefreshWidgetsFromState()
{
	const AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState)
	{
		return;
	}

	HandleScoreChanged(WAGameState->Score);
	HandleTimerUpdated(WAGameState->TimeLeft);
	HandleLivesChanged(WAGameState->Lives);
	HandleComboChanged(WAGameState->Combo, WAGameState->ComboProgress);
	UpdateBestScore(WAGameState->BestScore);
	if (!WAGameState->CurrentRound.Options.IsEmpty())
	{
		HandleRoundUpdated(WAGameState->CurrentRound);
	}
	HandleScreenChanged(WAGameState->CurrentScreen, WAGameState->bIsPaused, WAGameState->bIsGameActive);
}

void AWA_PlayerController::ShowOnlyMainMenu()
{
	SetWidgetVisibility(MainMenuWidget, true);
	SetWidgetVisibility(MainSettingsWidget, false);
	SetWidgetVisibility(MiniSettingsWidget, false);
	SetWidgetVisibility(PlayZoneWidget, false);
	SetWidgetVisibility(FailWidget, false);
	SetWidgetVisibility(NewScoreWidget, false);
}

void AWA_PlayerController::ShowMainSettings()
{
	SetWidgetVisibility(MainMenuWidget, false);
	SetWidgetVisibility(MainSettingsWidget, true);
	SetWidgetVisibility(MiniSettingsWidget, false);
	SetWidgetVisibility(PlayZoneWidget, false);
	SetWidgetVisibility(FailWidget, false);
	SetWidgetVisibility(NewScoreWidget, false);
}

void AWA_PlayerController::ShowPlaying()
{
	SetWidgetVisibility(MainMenuWidget, false);
	SetWidgetVisibility(MainSettingsWidget, false);
	SetWidgetVisibility(MiniSettingsWidget, false);
	SetWidgetVisibility(PlayZoneWidget, true);
	SetWidgetVisibility(FailWidget, false);
	SetWidgetVisibility(NewScoreWidget, false);
}

void AWA_PlayerController::ShowPause()
{
	SetWidgetVisibility(MainMenuWidget, false);
	SetWidgetVisibility(MainSettingsWidget, false);
	SetWidgetVisibility(PlayZoneWidget, true);
	SetWidgetVisibility(MiniSettingsWidget, true);
	SetWidgetVisibility(FailWidget, false);
	SetWidgetVisibility(NewScoreWidget, false);
}

void AWA_PlayerController::ShowGameOverWidgets(bool bNewBestScore)
{
	StopTimerWaitingAnimation();
	bPendingFailAfterNewScore = bNewBestScore;
	SetWidgetVisibility(MainMenuWidget, false);
	SetWidgetVisibility(MainSettingsWidget, false);
	SetWidgetVisibility(MiniSettingsWidget, false);
	SetWidgetVisibility(PlayZoneWidget, false);
	SetWidgetVisibility(FailWidget, !bNewBestScore);
	SetWidgetVisibility(NewScoreWidget, bNewBestScore);
}

void AWA_PlayerController::ShowFailOnly()
{
	StopTimerWaitingAnimation();
	SetWidgetVisibility(MainMenuWidget, false);
	SetWidgetVisibility(MainSettingsWidget, false);
	SetWidgetVisibility(MiniSettingsWidget, false);
	SetWidgetVisibility(PlayZoneWidget, false);
	SetWidgetVisibility(NewScoreWidget, false);
	SetWidgetVisibility(FailWidget, true);
}

void AWA_PlayerController::SetWidgetVisibility(UUserWidget* Widget, bool bVisible) const
{
	if (Widget)
	{
		Widget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void AWA_PlayerController::UpdateBestScore(int32 BestScore)
{
	SetText(PlayZoneWidget, TEXT("Text_ScoreCount_1"), FText::AsNumber(BestScore));
}

void AWA_PlayerController::UpdateOptionClicked(int32 OptionIndex)
{
	if (OptionWords.IsValidIndex(OptionIndex))
	{
		HandleWordSelected(OptionWords[OptionIndex]);
	}
}

void AWA_PlayerController::CachePlayZoneControls()
{
	OptionButtons.Reset();
	OptionTexts.Reset();

	UWidget* WordsRoot = FindNamedWidget<UWidget>(PlayZoneWidget, TEXT("HorizontalBox_Words"));
	if (UPanelWidget* WordsPanel = Cast<UPanelWidget>(WordsRoot))
	{
		for (int32 Index = 0; Index < WordsPanel->GetChildrenCount(); ++Index)
		{
			UWidget* SlotRoot = WordsPanel->GetChildAt(Index);
			OptionButtons.Add(FindFirstDescendantOfClass<UButton>(SlotRoot));
			OptionTexts.Add(FindFirstDescendantOfClass<UTextBlock>(SlotRoot));
		}
	}

	CacheHeartImages();
}

void AWA_PlayerController::CacheHeartImages()
{
	HeartImages.Reset();

	HeartImages.Add(FindNamedWidget<UImage>(PlayZoneWidget, TEXT("Image_Heart_1")));
	HeartImages.Add(FindNamedWidget<UImage>(PlayZoneWidget, TEXT("Image_Heart_2")));
	HeartImages.Add(FindNamedWidget<UImage>(PlayZoneWidget, TEXT("Image_Heart_3")));

	const bool bExactNamesFound = HeartImages.Num() == 3 && HeartImages[0] && HeartImages[1] && HeartImages[2];
	if (!bExactNamesFound)
	{
		UE_LOG(LogWAPlayerController, Warning, TEXT("CacheHeartImages: exact Image_Heart_1/2/3 lookup failed. Dumping all images and trying fallback."));

		HeartImages.Reset();
		TArray<UWidget*> AllPlayWidgets;
		CollectDescendants(PlayZoneWidget, AllPlayWidgets);

		for (UWidget* Widget : AllPlayWidgets)
		{
			UImage* Image = Cast<UImage>(Widget);
			if (!Image)
			{
				continue;
			}

			UObject* Resource = Image->GetBrush().GetResourceObject();
			const FString WidgetName = Image->GetName();
			const FString ResourcePath = Resource ? Resource->GetPathName() : FString();
			UE_LOG(LogWAPlayerController, Log, TEXT("CacheHeartImages: found image Name=%s Resource=%s"), *WidgetName, ResourcePath.IsEmpty() ? TEXT("none") : *ResourcePath);

			const bool bLooksLikeHeart =
				WidgetName.Contains(TEXT("Heart"), ESearchCase::IgnoreCase) ||
				WidgetName.Contains(TEXT("Hesrt"), ESearchCase::IgnoreCase) ||
				ResourcePath.Contains(TEXT("Heart"), ESearchCase::IgnoreCase);

			if (bLooksLikeHeart)
			{
				HeartImages.Add(Image);
			}
		}

		if (HeartImages.Num() > 3)
		{
			HeartImages.SetNum(3);
		}
	}

	for (int32 Index = 0; Index < HeartImages.Num(); ++Index)
	{
		UE_LOG(LogWAPlayerController, Log, TEXT("CachePlayZoneControls: HeartImages[%d]=%s"), Index, HeartImages[Index] ? *HeartImages[Index]->GetName() : TEXT("NOT FOUND"));
	}

	HeartLossTimerHandles.SetNum(HeartImages.Num());
	HeartLossFrameIndices.Init(0, HeartImages.Num());
}

void AWA_PlayerController::UpdateBackgroundByCombo(int32 Combo)
{
	if (BackgroundTextures.IsEmpty())
	{
		return;
	}

	if (Combo <= 0)
	{
		return;
	}

	if (Combo % 5 == 0)
	{
		SetPlayZoneBackground(FMath::Clamp(CurrentBackgroundIndex + 1, 0, BackgroundTextures.Num() - 1));
	}
}

void AWA_PlayerController::ResetPlayZoneBackground()
{
	CurrentBackgroundIndex = INDEX_NONE;
	bAlreadyNightSoundPlayed = false;
	SetPlayZoneBackground(0);
}

void AWA_PlayerController::SetPlayZoneBackground(int32 BackgroundIndex)
{
	if (!BackgroundTextures.IsValidIndex(BackgroundIndex) || !BackgroundTextures[BackgroundIndex])
	{
		return;
	}

	if (UBorder* BackgroundBorder = FindNamedWidget<UBorder>(PlayZoneWidget, TEXT("Border_BackGround_1")))
	{
		if (CurrentBackgroundIndex == BackgroundIndex)
		{
			return;
		}

		BackgroundBorder->SetBrushFromTexture(BackgroundTextures[BackgroundIndex]);
		CurrentBackgroundIndex = BackgroundIndex;

		if (BackgroundIndex == 6 && !bAlreadyNightSoundPlayed && AlreadyNightSound)
		{
			bAlreadyNightSoundPlayed = true;
			UGameplayStatics::PlaySound2D(this, AlreadyNightSound);
		}
	}
}

void AWA_PlayerController::UpdateHeartSprites(int32 Lives)
{
	if (HeartImages.IsEmpty())
	{
		CachePlayZoneControls();
	}

	const int32 NewLostCount = GetLostHeartCount(Lives);
	const int32 PreviousLostCount = GetLostHeartCount(LastDisplayedLives);
	UE_LOG(LogWAPlayerController, Log, TEXT("UpdateHeartSprites: Lives=%d PreviousLives=%d PreviousLost=%d NewLost=%d HeartImages=%d"), Lives, LastDisplayedLives, PreviousLostCount, NewLostCount, HeartImages.Num());

	if (Lives >= LastDisplayedLives)
	{
		UE_LOG(LogWAPlayerController, Log, TEXT("UpdateHeartSprites: lives increased/reset, applying direct state."));
		for (FTimerHandle& TimerHandle : HeartLossTimerHandles)
		{
			if (GetWorld())
			{
				GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			}
		}

		for (int32 Index = 0; Index < HeartImages.Num(); ++Index)
		{
			UE_LOG(LogWAPlayerController, Log, TEXT("UpdateHeartSprites direct: HeartIndex=%d State=%s"), Index, Index < NewLostCount ? TEXT("LostSprite") : TEXT("Alive"));
			if (Index < NewLostCount)
			{
				SetHeartSprite(HeartImages[Index], HeartLostSprite);
			}
			else
			{
				SetHeartSprite(HeartImages[Index], HeartAliveSprite);
			}
		}
		LastDisplayedLives = Lives;
		return;
	}

	for (int32 Index = 0; Index < HeartImages.Num(); ++Index)
	{
		if (Index < PreviousLostCount)
		{
			UE_LOG(LogWAPlayerController, Log, TEXT("UpdateHeartSprites existing lost: HeartIndex=%d"), Index);
			SetHeartSprite(HeartImages[Index], HeartLossSprites.IsValidIndex(18) ? HeartLossSprites[18] : HeartLostSprite);
		}
		else if (Index >= NewLostCount)
		{
			UE_LOG(LogWAPlayerController, Log, TEXT("UpdateHeartSprites still alive: HeartIndex=%d"), Index);
			SetHeartSprite(HeartImages[Index], HeartAliveSprite);
		}
	}

	for (int32 Index = PreviousLostCount; Index < NewLostCount; ++Index)
	{
		UE_LOG(LogWAPlayerController, Log, TEXT("UpdateHeartSprites transition requested: HeartIndex=%d"), Index);
		StartHeartLossTransition(Index);
	}

	LastDisplayedLives = Lives;
}

void AWA_PlayerController::SetHeartSprite(UImage* Image, UPaperSprite* Sprite) const
{
	if (!Image || !Sprite)
	{
		UE_LOG(LogWAPlayerController, Warning, TEXT("SetHeartSprite skipped: Image=%s Sprite=%s"), Image ? *Image->GetName() : TEXT("nullptr"), Sprite ? *Sprite->GetPathName() : TEXT("nullptr"));
		return;
	}

	UE_LOG(LogWAPlayerController, Log, TEXT("SetHeartSprite: Image=%s Sprite=%s"), *Image->GetName(), *Sprite->GetPathName());
	TScriptInterface<ISlateTextureAtlasInterface> AtlasRegion;
	AtlasRegion.SetObject(Sprite);
	AtlasRegion.SetInterface(Cast<ISlateTextureAtlasInterface>(Sprite));
	Image->SetBrushFromAtlasInterface(AtlasRegion, true);
}

void AWA_PlayerController::StartHeartLossTransition(int32 HeartIndex)
{
	if (!HeartImages.IsValidIndex(HeartIndex) || !HeartImages[HeartIndex] || !GetWorld())
	{
		UE_LOG(LogWAPlayerController, Warning, TEXT("StartHeartLossTransition skipped: HeartIndex=%d IsValid=%s Image=%s World=%s"),
			HeartIndex,
			HeartImages.IsValidIndex(HeartIndex) ? TEXT("true") : TEXT("false"),
			HeartImages.IsValidIndex(HeartIndex) && HeartImages[HeartIndex] ? *HeartImages[HeartIndex]->GetName() : TEXT("nullptr"),
			GetWorld() ? TEXT("valid") : TEXT("nullptr"));
		return;
	}

	UE_LOG(LogWAPlayerController, Log, TEXT("StartHeartLossTransition: HeartIndex=%d Image=%s FrameDelay=0.05"), HeartIndex, *HeartImages[HeartIndex]->GetName());
	if (HeartLossFrameIndices.IsValidIndex(HeartIndex))
	{
		HeartLossFrameIndices[HeartIndex] = 0;
	}
	SetHeartSprite(HeartImages[HeartIndex], HeartLossSprites.IsValidIndex(0) ? HeartLossSprites[0] : HeartAliveSprite);

	if (HeartLossTimerHandles.IsValidIndex(HeartIndex))
	{
		GetWorld()->GetTimerManager().ClearTimer(HeartLossTimerHandles[HeartIndex]);
		GetWorld()->GetTimerManager().SetTimer(
			HeartLossTimerHandles[HeartIndex],
			FTimerDelegate::CreateUObject(this, &AWA_PlayerController::AdvanceHeartLossFrame, HeartIndex),
			0.05f,
			true);
		UE_LOG(LogWAPlayerController, Log, TEXT("StartHeartLossTransition: timer set for HeartIndex=%d"), HeartIndex);
	}
	else
	{
		UE_LOG(LogWAPlayerController, Warning, TEXT("StartHeartLossTransition: no timer handle slot for HeartIndex=%d"), HeartIndex);
	}
}

void AWA_PlayerController::AdvanceHeartLossFrame(int32 HeartIndex)
{
	if (!HeartImages.IsValidIndex(HeartIndex) || !HeartImages[HeartIndex] || !HeartLossFrameIndices.IsValidIndex(HeartIndex))
	{
		UE_LOG(LogWAPlayerController, Warning, TEXT("AdvanceHeartLossFrame skipped: HeartIndex=%d"), HeartIndex);
		return;
	}

	HeartLossFrameIndices[HeartIndex]++;
	const int32 FrameIndex = FMath::Clamp(HeartLossFrameIndices[HeartIndex], 0, 18);
	UE_LOG(LogWAPlayerController, Log, TEXT("AdvanceHeartLossFrame: HeartIndex=%d FrameIndex=%d Image=%s"),
		HeartIndex,
		FrameIndex,
		*HeartImages[HeartIndex]->GetName());

	SetHeartSprite(HeartImages[HeartIndex], HeartLossSprites.IsValidIndex(FrameIndex) ? HeartLossSprites[FrameIndex] : HeartLostSprite);

	if (FrameIndex >= 18 && GetWorld() && HeartLossTimerHandles.IsValidIndex(HeartIndex))
	{
		GetWorld()->GetTimerManager().ClearTimer(HeartLossTimerHandles[HeartIndex]);
		UE_LOG(LogWAPlayerController, Log, TEXT("AdvanceHeartLossFrame: completed HeartIndex=%d"), HeartIndex);
	}
}

int32 AWA_PlayerController::GetLostHeartCount(int32 Lives) const
{
	return FMath::Clamp(3 - Lives, 0, 3);
}

void AWA_PlayerController::PlayWidgetAnimationByName(UUserWidget* Widget, FName AnimationName, bool bLoop)
{
	if (!Widget)
	{
		return;
	}

	if (UWidgetAnimation* Animation = FindWidgetAnimationByName(Widget, AnimationName))
	{
		if (bLoop && Widget->IsAnimationPlaying(Animation))
		{
			return;
		}

		Widget->PlayAnimation(Animation, 0.0f, bLoop ? 0 : 1, EUMGSequencePlayMode::Forward, 1.0f);
	}
}

void AWA_PlayerController::StopWidgetAnimationByName(UUserWidget* Widget, FName AnimationName)
{
	if (!Widget)
	{
		return;
	}

	if (UWidgetAnimation* Animation = FindWidgetAnimationByName(Widget, AnimationName))
	{
		if (Widget->IsAnimationPlaying(Animation))
		{
			Widget->StopAnimation(Animation);
		}
	}
}

UWidgetAnimation* AWA_PlayerController::FindWidgetAnimationByName(UUserWidget* Widget, FName AnimationName) const
{
	if (!Widget)
	{
		return nullptr;
	}

	const FString DesiredName = AnimationName.ToString();
	for (TFieldIterator<FObjectProperty> PropertyIt(Widget->GetClass()); PropertyIt; ++PropertyIt)
	{
		FObjectProperty* ObjectProperty = *PropertyIt;
		if (!ObjectProperty || !ObjectProperty->PropertyClass->IsChildOf(UWidgetAnimation::StaticClass()))
		{
			continue;
		}

		UWidgetAnimation* Animation = Cast<UWidgetAnimation>(ObjectProperty->GetObjectPropertyValue_InContainer(Widget));
		if (!Animation)
		{
			continue;
		}

		const bool bPropertyMatches = ObjectProperty->GetName().Contains(DesiredName);
		const bool bAnimationMatches = Animation->GetName().Contains(DesiredName);
		const bool bMovieSceneMatches = Animation->GetMovieScene() && Animation->GetMovieScene()->GetName().Contains(DesiredName);
		if (bPropertyMatches || bAnimationMatches || bMovieSceneMatches)
		{
			return Animation;
		}
	}

	return nullptr;
}

void AWA_PlayerController::StartTimerWaitingAnimation()
{
	PlayWidgetAnimationByName(PlayZoneWidget, TEXT("Timer_Anim_Waiting"), true);
}

void AWA_PlayerController::StopTimerWaitingAnimation()
{
	StopWidgetAnimationByName(PlayZoneWidget, TEXT("Timer_Anim_Waiting"));
}

void AWA_PlayerController::PlayTimerNewTimeAnimation()
{
	PlayWidgetAnimationByName(PlayZoneWidget, TEXT("Timer_Anim_NewTime"), false);
}

AWA_GameMode* AWA_PlayerController::GetWAGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AWA_GameMode>() : nullptr;
}

AWA_GameState* AWA_PlayerController::GetWAGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWA_GameState>() : nullptr;
}
