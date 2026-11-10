// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDGameMode.h"
#include "SSDPawn.h"
#include "SSDPawn_Transformed.h"
#include "SSDMovementComponent.h"
#include "SSDStageBounds.h"
#include "SSDCamera.h"
#include "SSDGameInstance.h"
#include "Input/SSDAIController.h"
#include "Input/SSDUnassignedPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "SSDWorldSettings.h"
#include "PawnRenderLayerComponent.h"


ASSDGameMode::ASSDGameMode()
{
	PlayerControllerClass = ASSDPlayerController::StaticClass();
}

void ASSDGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get World."));
		return;
	}

	USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve GameInstance."));
		return;
	}

	TSubclassOf<ASSDPawn> P1Char = GameInstance->P1Char;
	TSubclassOf<ASSDPawn> P2Char = GameInstance->P2Char;
	TSubclassOf<ASSDPawn_Transformed> P1Transformed = GameInstance->P1Transformed;
	TSubclassOf<ASSDPawn_Transformed> P2Transformed = GameInstance->P2Transformed;

	if (!P1Char || !P2Char || !P1Transformed || !P2Transformed)
	{
		UE_LOG(LogTemp, Error, TEXT("Some pawn slots are empty. Check the match GameMode Blueprint."));
		return;
	}

	TArray<AActor*> StageBoundsList;
	ASSDStageBounds* SBActor;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASSDStageBounds::StaticClass(), StageBoundsList);
	if (StageBoundsList.IsValidIndex(0))
	{
		if (StageBoundsList[0])
		{
			SBActor = Cast<ASSDStageBounds>(StageBoundsList[0]);
			if (!SBActor)
			{
				UE_LOG(LogTemp, Error, TEXT("No stage bounds found in the level."));
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No stage bounds found in the level."));
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No stage bounds found in the level."));
		return;
	}


	// Wipe all the last PCs and Pawns.
	TArray<AActor*> AllPCs;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), AllPCs);
	for (int i = 0; i < AllPCs.Num(); i++)
	{
		APlayerController* PCi = Cast<APlayerController>(AllPCs[i]);
		UGameplayStatics::RemovePlayer(PCi, true);
	}

	// Spawn first player.
	DefaultPawnClass = P1Char;
	
	// Always spawn PC1 even if CPUvsCPU, since the camera needs it.
	APlayerController* PC1 = UGameplayStatics::CreatePlayer(GetWorld(), GameInstance->P1ControllerID);
	if (!PC1)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn PC 1"));
		return;
	}

	if (GameInstance->P1ControllerID < 0)
	{
		GameInstance->P1ControllerID = 0;
	}

	if (GameInstance->bCPUVersusCPU)
	{
		PC1->GetPawn()->Destroy();

		Fighter1 = Cast<ASSDPawn>(World->SpawnActor(DefaultPawnClass));
		ASSDAIController* AIController = Fighter1->GetController<ASSDAIController>();
		if (!AIController)
		{
			AIController = World->SpawnActor<ASSDAIController>();
			if(AIController)
			{
				AIController->Possess(Fighter1);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn CPU 1"));
				return;
			}
		}
		
		AIController->Action = ESSDAIAction::SSDAIA_CPU;
		AIController->AILevel = GameInstance->AILevel;
	}
	else
	{
		Fighter1 = Cast<ASSDPawn>(PC1->GetPawn());
	}
	if (!Fighter1)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Pawn 1"));
		return;
	}
	Fighter1->SetActorLocation(SBActor->GetActorLocation() - (SBActor->GetActorForwardVector() * (SBActor->SpawnDistance - Fighter1->Capsule->GetScaledCapsuleRadius())) 
		+ (SBActor->GetActorUpVector() * Fighter1->Capsule->GetScaledCapsuleHalfHeight()));
	Fighter1->SetActorRotation(SBActor->GetActorRotation());
	Fighter1->PlayerIndex = 0;
	Fighter1->BringToFrontDepthLayer();
	Fighter1->MovementComponent->StageLocation = SBActor->GetActorLocation();
	Fighter1->TransformGain = GameInstance->TransformGain;
	Fighter1->SuperGain = GameInstance->SuperGain;

	// Spawn transformed P1.
	Fighter1->Transformed = GetWorld()->SpawnActor<ASSDPawn_Transformed>(P1Transformed);
	if (!Fighter1->Transformed)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Transformed Pawn 1"));
		return;
	}
	ASSDPawn_Transformed* F1T = Cast<ASSDPawn_Transformed>(Fighter1->Transformed);
	F1T->Normal = Fighter1;
	F1T->SetActorHiddenInGame(true);
	if (F1T->RenderLayer && Fighter1->RenderLayer)
		F1T->RenderLayer->SetSharedGroup(Fighter1->RenderLayer);
	F1T->SetActorTickEnabled(false);
	F1T->MovementComponent->SetComponentTickEnabled(false);
	F1T->SetActorLocation(SBActor->GetActorLocation() + ((SBActor->CeilingHeight + 500) * FVector::UpVector));
	F1T->SetActorRotation(SBActor->GetActorRotation());
	F1T->PlayerIndex = 0;
	F1T->BringToFrontDepthLayer();
	F1T->MovementComponent->StageLocation = SBActor->GetActorLocation();
	F1T->TransformGain = GameInstance->TransformGain;
	F1T->SuperGain = GameInstance->SuperGain;

	// Spawn second player.
	DefaultPawnClass = P2Char;

	APlayerController *PC2;
	if (GameInstance->IsSinglePlayerMode())
	{
		Fighter2 = Cast<ASSDPawn>(World->SpawnActor(DefaultPawnClass));
		ASSDAIController* AIController = Fighter2->GetController<ASSDAIController>();
		if ((GameInstance->bVersusCPU || GameInstance->bCPUVersusCPU) && AIController)
		{
			AIController->Action = ESSDAIAction::SSDAIA_CPU;
			AIController->AILevel = GameInstance->AILevel;
		}
	}
	else
	{
		if (GameInstance->P2ControllerID < 0)
		{
			GameInstance->P2ControllerID = 1;
		}

		PC2 = UGameplayStatics::CreatePlayer(GetWorld(), GameInstance->P2ControllerID);
		if (!PC2)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn PC 2"));
			return;
		}

		Fighter2 = Cast<ASSDPawn>(PC2->GetPawn());
	}
	if (!Fighter2)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Pawn 2"));
		return;
	}
	Fighter2->SetActorLocation(SBActor->GetActorLocation() + (SBActor->GetActorForwardVector() * (SBActor->SpawnDistance - Fighter2->Capsule->GetScaledCapsuleRadius()))
		+ (SBActor->GetActorUpVector() * Fighter1->Capsule->GetScaledCapsuleHalfHeight()));
	Fighter2->SetActorRotation(SBActor->GetActorRotation());
	Fighter2->PlayerIndex = 1;
	Fighter2->SendToBackDepthLayer();
	Fighter2->MovementComponent->StageLocation = SBActor->GetActorLocation();
	// Force P2 to look left.
	Fighter2->MovementComponent->bShouldFaceRight = false;
	Fighter2->SetFacing(false);
	Fighter2->TransformGain = GameInstance->TransformGain;
	Fighter2->SuperGain = GameInstance->SuperGain;
	
	// Spawn transformed P2.
	Fighter2->Transformed = GetWorld()->SpawnActor<ASSDPawn_Transformed>(P2Transformed);
	if (!Fighter2->Transformed)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Transformed Pawn 2"));
		return;
	}
	ASSDPawn_Transformed* F2T = Cast<ASSDPawn_Transformed>(Fighter2->Transformed);
	F2T->Normal = Fighter2;
	F2T->SetActorHiddenInGame(true);
	if (F2T->RenderLayer && Fighter2->RenderLayer)
		F2T->RenderLayer->SetSharedGroup(Fighter2->RenderLayer);
	F2T->SetActorTickEnabled(false);
	F2T->MovementComponent->SetComponentTickEnabled(false);
	F2T->SetActorLocation(SBActor->GetActorLocation() + ((SBActor->CeilingHeight + 500) * FVector::UpVector));
	F2T->SetActorRotation(SBActor->GetActorRotation());
	F2T->PlayerIndex = 1;
	F2T->SendToBackDepthLayer();
	F2T->MovementComponent->StageLocation = SBActor->GetActorLocation();
	F2T->TransformGain = GameInstance->TransformGain;
	F2T->SuperGain = GameInstance->SuperGain;

	Fighter1->Opponent = Fighter2;
	Fighter2->Opponent = Fighter1;

	Fighter1->GameMode = this;
	Fighter2->GameMode = this;
	F1T->GameMode = this;
	F2T->GameMode = this;

	// Spawn additional players to represent the rest of the controllers, to check them for inputs
	UnassignedControllers = TArray<APlayerController*>();
	DefaultPawnClass = nullptr;
	TSubclassOf<APlayerController> DefaultController = PlayerControllerClass;
	PlayerControllerClass = ASSDUnassignedPlayerController::StaticClass();
	for (size_t i = 0; i < 8; i++)
	{
		if (GameInstance->P1ControllerID == i)
			if (!GameInstance->bCPUVersusCPU)
				continue;
		// Skip if player 2 is real
		if (GameInstance->P2ControllerID == i)
			if (!GameInstance->IsSinglePlayerMode())
				continue;

		UnassignedControllers.Add(UGameplayStatics::CreatePlayer(GetWorld(), i));
	}
	PlayerControllerClass = DefaultController;

	Fighter1->MovementComponent->MaxDistanceToOpponent = SBActor->MaxDistance;
	Fighter2->MovementComponent->MaxDistanceToOpponent = SBActor->MaxDistance;
	F1T->MovementComponent->MaxDistanceToOpponent = SBActor->MaxDistance;
	F2T->MovementComponent->MaxDistanceToOpponent = SBActor->MaxDistance;

	Camera = GetWorld()->SpawnActor<ASSDCamera>();
	if (!Camera)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load camera."));
		return;
	}
	Fighter1->Camera = Camera;
	Fighter2->Camera = Camera;
	F1T->Camera = Camera;
	F2T->Camera = Camera;

	ShowHitboxes(GameInstance->bHitboxesVisible);

	ASSDWorldSettings* Settings = Cast<ASSDWorldSettings>(World->GetWorldSettings());
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("Please assign 'SSDWorldSettings' as the default WorldSettings class."));
		return;
	}

	Camera->Init(SSDCameraSettings(
		Fighter1,
		Fighter2,
		Settings->CameraSettings.FoV,
		Settings->CameraSettings.HScrollPadding,
		Settings->CameraSettings.VScrollPadding,
		Settings->CameraSettings.MinDistance,
		Settings->CameraSettings.MinHeight,
		SBActor->HorizontalBoundsHalfExtent,
		SBActor->CeilingHeight,
		Settings->CameraSettings.Speed,
		Settings->CameraSettings.ZoomPitchBias,
		Settings->CameraSettings.ZoomLiftBias,
		Settings->CameraSettings.ShakeOrbitOffset,
		Settings->CameraSettings.MaxHitShake,
		Settings->CameraSettings.HitShakeSpeed,
		Settings->CameraSettings.HitShakeTime,
		Settings->CameraSettings.MaxBounceShake,
		Settings->CameraSettings.BounceShakeSpeed,
		Settings->CameraSettings.BounceShakeTime
	));

	PC1->SetViewTarget(Camera);

	if (GameInstance->bTrainingMode)
	{
		Fighter1->TrainingRefill();
		Fighter2->TrainingRefill();
	}

	// Acknowledge BP.
	Initialized();
}

ASSDPawn * ASSDGameMode::GetPlayer(int Index) const
{
	switch (Index)
	{
	case 0:
		return Fighter1;
		break;

	case 1:
		return Fighter2;
		break;

	default:
		return nullptr;
		break;
	}
}

FVector2D ASSDGameMode::GetP1ScreenPos() const
{
	if (!Camera) return FVector2D::ZeroVector;

	return Camera->GetP1ScreenPos();
}

FVector2D ASSDGameMode::GetP2ScreenPos() const
{
	if (!Camera) return FVector2D::ZeroVector;

	return Camera->GetP2ScreenPos();
}

bool ASSDGameMode::SetControllerId(int PlayerIndex, int NewControllerId)
{
	USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	int* PlayerID, * OtherPlayerID;
	ASSDPawn* Fighter = GetPlayer(PlayerIndex);
	if (Fighter->SetControllerId(NewControllerId))
	{
		if (Fighter == Fighter1)
		{
			PlayerID = &GameInstance->P1ControllerID;
			OtherPlayerID = &GameInstance->P2ControllerID;
		}
		else
		{
			PlayerID = &GameInstance->P2ControllerID;
			OtherPlayerID = &GameInstance->P1ControllerID;
		}

		// If the other player was using that controller id, they need swapped
		if (*OtherPlayerID == NewControllerId)
			*OtherPlayerID = *PlayerID;
		*PlayerID = NewControllerId;

		return true;
	}

	return false;
}

void ASSDGameMode::UnassignedControllerInput(ASSDUnassignedPlayerController* Controller)
{
	USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	// If in training mode
	if (GameInstance->bTrainingMode)
	{
		// If the training dummy wants a controller and this one is valid
		ASSDAIController* AIController = Fighter2->GetController<ASSDAIController>();
		if (AIController->WantsPlayerController() && UnassignedControllers.Contains(Controller))
		{
			int ControllerId = Controller->GetLocalPlayer()->GetControllerId();
			// Remove the old controller
			UnassignedControllers.Remove(Controller);
			UGameplayStatics::RemovePlayer(Controller, false);

			// Create a new player for the new controller
			DefaultPawnClass = nullptr;
			ASSDPlayerController* NewController = Cast<ASSDPlayerController>(UGameplayStatics::CreatePlayer(GetWorld(), ControllerId));

			if (!NewController)
			{
				UE_LOG(LogTemp, Error, TEXT("Created a player controller of the wrong class."));
			}

			AIController->SetPlayer(NewController);
		}
	}
}

void ASSDGameMode::UnassignController(ASSDPlayerController* Controller)
{
	int ControllerId = Controller->GetLocalPlayer()->GetControllerId();
	// Remove the old controller
	UGameplayStatics::RemovePlayer(Controller, false);

	// Spawn unassigned player controller
	TSubclassOf<APlayerController> DefaultController = PlayerControllerClass;
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ASSDUnassignedPlayerController::StaticClass();
	UnassignedControllers.Add(UGameplayStatics::CreatePlayer(GetWorld(), ControllerId));
	PlayerControllerClass = DefaultController;
}

void ASSDGameMode::ShowHitboxes(bool Visible)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get World."));
		return;
	}

	USSDGameInstance* GameInstance = Cast<USSDGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve GameInstance."));
		return;
	}

	if (GameInstance->bTrainingMode)
	{
		GameInstance->bHitboxesVisible = Visible;

		Fighter1->HitboxVisibility(GameInstance->bHitboxesVisible);
		if (Fighter1->Transformed) Fighter1->Transformed->HitboxVisibility(GameInstance->bHitboxesVisible);
		else if (ASSDPawn_Transformed* T = Cast<ASSDPawn_Transformed>(Fighter1)) T->Normal->HitboxVisibility(GameInstance->bHitboxesVisible);

		Fighter2->HitboxVisibility(GameInstance->bHitboxesVisible);
		if (Fighter2->Transformed) Fighter2->Transformed->HitboxVisibility(GameInstance->bHitboxesVisible);
		else if (ASSDPawn_Transformed* T = Cast<ASSDPawn_Transformed>(Fighter2)) T->Normal->HitboxVisibility(GameInstance->bHitboxesVisible);
	}
}

void ASSDGameMode::TrainingRefill()
{
	Fighter1->TrainingRefill();
	Fighter2->TrainingRefill();
}

void ASSDGameMode::Initialized_Implementation()
{

}
