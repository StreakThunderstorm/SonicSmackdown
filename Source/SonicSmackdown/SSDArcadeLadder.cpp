// Fill out your copyright notice in the Description page of Project Settings.


#include "SSDArcadeLadder.h"
#include "SSDPawn.h"
#include "Kismet/GameplayStatics.h"
#include "SSDGameInstance.h"
//#include "GameplayStatics.generated.h"

USSDArcadeLadder::USSDArcadeLadder()
{
	ArcadeIndex = 0;
}

void USSDArcadeLadder::Initialize()
{
    Opponents = GetPossibleOpponents();
}

TArray<TSubclassOf<ASSDPawn>> USSDArcadeLadder::GetPossibleOpponents_Implementation()
{
	// Should override in all blueprint subclasses
	return TArray<TSubclassOf<ASSDPawn>>();
}

USSDGameInstance* USSDArcadeLadder::GetGameInstance() const
{
    UWorld *World = GetWorld();
    if(!World) return nullptr;

    USSDGameInstance * GI = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(World));
    return GI;
}
TArray<TSubclassOf<ASSDPawn>> USSDArcadeLadder::DrawHand(TArray<TSubclassOf<ASSDPawn>> Deck, int Count, TSubclassOf<ASSDPawn> rivalPawn, TSubclassOf<ASSDPawn> bossPawn, TSubclassOf<ASSDPawn> MechaPawn)
{
	TArray<TSubclassOf<ASSDPawn>> Result;

    USSDGameInstance *pInstance = GetGameInstance();
    //pawnRival = pInstance->P1Char;
    int selfPawnID = 0;
    int rivalPawnID = 13;
    int MechaPawnID = 15;
    int bossPawnID = 14;

    for (int i = 0; i<Deck.Num(); i++)
    {
        if (Deck[i] == pInstance->P1Char)
        {
            selfPawnID = i;
            break;
        }
    }

    rivalPawn = Deck[rivalPawnID];
    bossPawn = Deck[bossPawnID];
    MechaPawn = Deck[MechaPawnID];

    for(int i=0;i < Deck.Num();i++)
    {
        if (Deck[i] == rivalPawn) Deck.RemoveAt(i);
        else  if (Deck[i] == bossPawn) Deck.RemoveAt(i);
        else  if (Deck[i] == MechaPawn) Deck.RemoveAt(i);
    }

    while (Result.Num() < Count && Deck.Num() > 0)
    {
        int Index = FMath::RandRange(0, Deck.Num() - 2); //@TODO: use noise instead of rng
        if (Deck[Index] != rivalPawn || Deck[Index] != bossPawn || Deck[Index] != MechaPawn)
        {
            Result.Add(Deck[Index]);
            Deck.RemoveAt(Index);
        }
    }

    Result.Add(rivalPawn);
    Result.Add(bossPawn);
    Result.Add(MechaPawn);
	return Result;
}

bool USSDArcadeLadder::IsFirstMatch() const
{
	return ArcadeIndex == 0;
}

bool USSDArcadeLadder::IsArcadeComplete() const
{
	return ArcadeIndex >= Opponents.Num();
}

void USSDArcadeLadder::MoveToNextOpponent()
{
	ArcadeIndex++;
}

TSubclassOf<ASSDPawn> USSDArcadeLadder::CurrentOpponent() const
{
	return Opponents[ArcadeIndex];
}

float USSDArcadeLadder::CurrentDifficulty() const
{
	float Alpha;
	if (Opponents.Num() <= 1)
		Alpha = 1.f;
	else
		Alpha = ArcadeIndex / (float)(Opponents.Num() - 1);

	return FMath::Lerp(BaseDifficulty, MaxDifficulty, Alpha);
}
