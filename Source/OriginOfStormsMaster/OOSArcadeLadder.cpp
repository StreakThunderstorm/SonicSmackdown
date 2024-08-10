// Fill out your copyright notice in the Description page of Project Settings.


#include "OOSArcadeLadder.h"
#include "OOSPawn.h"
#include "Kismet/GameplayStatics.h"
#include "OOSGameInstance.h"
//#include "GameplayStatics.generated.h"

UOOSArcadeLadder::UOOSArcadeLadder()
{
	ArcadeIndex = 0;
}

void UOOSArcadeLadder::Initialize()
{
    Opponents = GetPossibleOpponents();
}

TArray<TSubclassOf<AOOSPawn>> UOOSArcadeLadder::GetPossibleOpponents_Implementation()
{
	// Should override in all blueprint subclasses
	return TArray<TSubclassOf<AOOSPawn>>();
}

UOOSGameInstance* UOOSArcadeLadder::GetGameInstance() const
{
    UWorld *World = GetWorld();
    if(!World) return nullptr;

    UOOSGameInstance * GI = Cast<UOOSGameInstance>(UGameplayStatics::GetGameInstance(World));
    return GI;
}
TArray<TSubclassOf<AOOSPawn>> UOOSArcadeLadder::DrawHand(TArray<TSubclassOf<AOOSPawn>> Deck, int Count, TSubclassOf<AOOSPawn> rivalPawn, TSubclassOf<AOOSPawn> bossPawn, TSubclassOf<AOOSPawn> MechaPawn)
{
	TArray<TSubclassOf<AOOSPawn>> Result;

    UOOSGameInstance *pInstance = GetGameInstance();
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

bool UOOSArcadeLadder::IsFirstMatch() const
{
	return ArcadeIndex == 0;
}

bool UOOSArcadeLadder::IsArcadeComplete() const
{
	return ArcadeIndex >= Opponents.Num();
}

void UOOSArcadeLadder::MoveToNextOpponent()
{
	ArcadeIndex++;
}

TSubclassOf<AOOSPawn> UOOSArcadeLadder::CurrentOpponent() const
{
	return Opponents[ArcadeIndex];
}

float UOOSArcadeLadder::CurrentDifficulty() const
{
	float Alpha;
	if (Opponents.Num() <= 1)
		Alpha = 1.f;
	else
		Alpha = ArcadeIndex / (float)(Opponents.Num() - 1);

	return FMath::Lerp(BaseDifficulty, MaxDifficulty, Alpha);
}
