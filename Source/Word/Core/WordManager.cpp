// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WordManager.h"

void UWordManager::Initialize()
{
	BuildDefaultWordList();
	StartGame();
}

void UWordManager::StartGame()
{
	ShuffleWords();
	CurrentIndex = 0;
}

bool UWordManager::GenerateRound(FWordRound& OutRound)
{
	if (WordList.Num() < 5)
	{
		return false;
	}

	const FString CorrectWord = GetNextWord();
	const TArray<TCHAR> Letters = ExtractLetters(CorrectWord);
	if (Letters.Num() < 3)
	{
		return GenerateRound(OutRound);
	}

	TArray<FString> WrongOptions;
	TArray<FString> Candidates = WordList;
	for (int32 Index = Candidates.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		Candidates.Swap(Index, SwapIndex);
	}

	for (const FString& Candidate : Candidates)
	{
		if (Candidate != CorrectWord && !ContainsAllLetters(Candidate, Letters))
		{
			WrongOptions.Add(Candidate);
			if (WrongOptions.Num() == 4)
			{
				break;
			}
		}
	}

	if (WrongOptions.Num() < 4)
	{
		return false;
	}

	OutRound.CorrectWord = CorrectWord;
	OutRound.Letters = Letters;
	OutRound.Options.Reset();
	OutRound.Options.Add(CorrectWord);
	OutRound.Options.Append(WrongOptions);

	for (int32 Index = OutRound.Options.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		OutRound.Options.Swap(Index, SwapIndex);
	}

	return true;
}

void UWordManager::BuildDefaultWordList()
{
	if (!WordList.IsEmpty())
	{
		return;
	}

	WordList = {
		TEXT("АПЕЛЬСИН"), TEXT("БАРАБАН"), TEXT("БЕРЕЗКА"), TEXT("БИБЛИОТЕКА"), TEXT("БЛОКНОТ"),
		TEXT("БОТАНИКА"), TEXT("БУКВАРЬ"), TEXT("ВАГОНЕТКА"), TEXT("ВАРЕЖКА"), TEXT("ВЕЛОСИПЕД"),
		TEXT("ВЕРШИНА"), TEXT("ВИТРИНА"), TEXT("ВОРОТА"), TEXT("ГАЛАКТИКА"), TEXT("ГАРМОНИЯ"),
		TEXT("ГВОЗДИКА"), TEXT("ГЕРОЙСТВО"), TEXT("ГИТАРА"), TEXT("ГЛОБУС"), TEXT("ГОЛОВОЛОМКА"),
		TEXT("ГОРОДОК"), TEXT("ГРАМОТА"), TEXT("ГРАНИЦА"), TEXT("ДВОРЕЦ"), TEXT("ДЕКАБРЬ"),
		TEXT("ДЕРЕВНЯ"), TEXT("ДИРИЖАБЛЬ"), TEXT("ДОРОЖКА"), TEXT("ДРАКОН"), TEXT("ЖУРНАЛ"),
		TEXT("ЗАВТРАК"), TEXT("ЗАГАДКА"), TEXT("ЗВЕЗДА"), TEXT("ЗЕРКАЛО"), TEXT("ЗИМОРОДОК"),
		TEXT("ИГРУШКА"), TEXT("ИЗБУШКА"), TEXT("ИНЖЕНЕР"), TEXT("ИСТОРИЯ"), TEXT("КАБИНЕТ"),
		TEXT("КАЛЕНДАРЬ"), TEXT("КАМЕРА"), TEXT("КАПИТАН"), TEXT("КАРАНДАШ"), TEXT("КАРТИНА"),
		TEXT("КЛАВИША"), TEXT("КЛУБНИКА"), TEXT("КОЛЕСО"), TEXT("КОМАНДА"), TEXT("КОМПАС"),
		TEXT("КОРАБЛЬ"), TEXT("КОРОБКА"), TEXT("КОСМОНАВТ"), TEXT("КРЕПОСТЬ"), TEXT("КРИСТАЛЛ"),
		TEXT("КУЗНИЦА"), TEXT("ЛАБИРИНТ"), TEXT("ЛАМПОЧКА"), TEXT("ЛЕГЕНДА"), TEXT("ЛЕСНИЦА"),
		TEXT("ЛИМОНАД"), TEXT("ЛОКОМОТИВ"), TEXT("МАГАЗИН"), TEXT("МАЛИНА"), TEXT("МАРШРУТ"),
		TEXT("МАСТЕРСКАЯ"), TEXT("МЕДАЛЬОН"), TEXT("МЕЛОДИЯ"), TEXT("МОЗАИКА"), TEXT("МОЛНИЯ"),
		TEXT("НАВИГАТОР"), TEXT("НАРОДНОСТЬ"), TEXT("НЕБОСКЛОН"), TEXT("НОВЕЛЛА"), TEXT("ОБЛАКО"),
		TEXT("ОКЕАНАРИУМ"), TEXT("ОРБИТА"), TEXT("ПАЛИТРА"), TEXT("ПАРУСНИК"), TEXT("ПЕЩЕРА"),
		TEXT("ПИРАМИДА"), TEXT("ПЛАНЕТА"), TEXT("ПЛОЩАДЬ"), TEXT("ПОБЕДА"), TEXT("ПОДАРОК"),
		TEXT("ПРИКЛЮЧЕНИЕ"), TEXT("ПРОГРАММА"), TEXT("РАДУГА"), TEXT("РАКЕТА"), TEXT("РАССКАЗ"),
		TEXT("РЕКОРД"), TEXT("РЕМЕСЛО"), TEXT("РИСУНОК"), TEXT("РОМАШКА"), TEXT("САМОЛЕТ"),
		TEXT("САПОГИ"), TEXT("СЕКУНДА"), TEXT("СКАЗКА"), TEXT("СКРИПКА"), TEXT("СЛОВАРЬ"),
		TEXT("СНЕЖИНКА"), TEXT("СОКРОВИЩЕ"), TEXT("СТАДИОН"), TEXT("СТРАНИЦА"), TEXT("СУНДУК"),
		TEXT("ТЕЛЕСКОП"), TEXT("ТЕТРАДЬ"), TEXT("ТРОПИНКА"), TEXT("УЧЕБНИК"), TEXT("ФАНТАЗИЯ"),
		TEXT("ФЕВРАЛЬ"), TEXT("ФОНАРИК"), TEXT("ХУДОЖНИК"), TEXT("ЦВЕТОК"), TEXT("ЧЕМОДАН"),
		TEXT("ШАХМАТЫ"), TEXT("ШКОЛЬНИК"), TEXT("ЭКРАН"), TEXT("ЭНЕРГИЯ"), TEXT("ЯБЛОКО")
	};
}

void UWordManager::ShuffleWords()
{
	ShuffledWords = WordList;
	for (int32 Index = ShuffledWords.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		ShuffledWords.Swap(Index, SwapIndex);
	}
}

FString UWordManager::GetNextWord()
{
	if (ShuffledWords.IsEmpty())
	{
		ShuffleWords();
	}

	if (CurrentIndex >= ShuffledWords.Num())
	{
		ShuffleWords();
		CurrentIndex = 0;
	}

	return ShuffledWords[CurrentIndex++];
}

TArray<TCHAR> UWordManager::ExtractLetters(const FString& Word) const
{
	TArray<TCHAR> UniqueLetters;
	for (const TCHAR Character : Word)
	{
		if (!UniqueLetters.Contains(Character))
		{
			UniqueLetters.Add(Character);
		}
	}

	for (int32 Index = UniqueLetters.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		UniqueLetters.Swap(Index, SwapIndex);
	}

	if (UniqueLetters.Num() > 3)
	{
		UniqueLetters.SetNum(3);
	}

	return UniqueLetters;
}

bool UWordManager::ContainsAllLetters(const FString& Word, const TArray<TCHAR>& Letters) const
{
	for (const TCHAR Letter : Letters)
	{
		if (!Word.Contains(FString::Chr(Letter)))
		{
			return false;
		}
	}

	return true;
}
