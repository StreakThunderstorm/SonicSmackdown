#include "SSDInputProbe.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InputComponent.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "GameFramework/PlayerInput.h"
#include "Input/SSDFighterInputs.h"
#include "Input/SSDPlayerController.h"
#include "InputKeyEventArgs.h"
#include "PlayInEditorDataTypes.h"
#include "Serialization/JsonSerializer.h"
#include "SkeletalAnimation/SSDSkeletalMeshComponent.h"
#include "SSDGameInstance.h"
#include "SSDGameMode.h"
#include "SSDMovementComponent.h"
#include "SSDPawn.h"
#include "UObject/UnrealType.h"

namespace SSDInputProbe
{
	bool IsEnabled()
	{
		return IsInGameThread() && FParse::Param(FCommandLine::Get(), TEXT("SSDInputProbe"));
	}

	UWorld* GetPlayWorld()
	{
		if (!IsEnabled() || !GEngine)
			return nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
			if (Context.WorldType == EWorldType::PIE && Context.World())
				return Context.World();
		return nullptr;
	}

	FString Serialize(const TSharedRef<FJsonObject>& Object)
	{
		FString Result;
		FJsonSerializer::Serialize(Object, TJsonWriterFactory<>::Create(&Result));
		return Result;
	}

	TSharedRef<FJsonObject> DescribeTransform(const FTransform& Transform)
	{
		TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetStringField(TEXT("location"), Transform.GetLocation().ToString());
		Result->SetStringField(TEXT("rotation"), Transform.GetRotation().Rotator().ToString());
		Result->SetStringField(TEXT("quaternion"), Transform.GetRotation().ToString());
		Result->SetStringField(TEXT("scale"), Transform.GetScale3D().ToString());
		return Result;
	}

	TSharedRef<FJsonObject> DescribeAuthoredProperties(UObject* Object)
	{
		TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		if (!Object)
			return Result;
		for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
		{
			FProperty* Property = *It;
			UClass* OwnerClass = Cast<UClass>(Property->GetOwnerStruct());
			if (!OwnerClass || !OwnerClass->GetPathName().StartsWith(TEXT("/Game/")) || Property->IsA<FMulticastDelegateProperty>())
				continue;
			FString Value;
			Property->ExportTextItem_Direct(Value, Property->ContainerPtrToValuePtr<void>(Object), nullptr, Object, PPF_None);
			Result->SetStringField(Property->GetName(), Value.Left(2048));
		}
		return Result;
	}

	TSharedRef<FJsonObject> DescribeMesh(USkeletalMeshComponent* Mesh, bool bIncludeReference)
	{
		TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetStringField(TEXT("path"), GetPathNameSafe(Mesh));
		if (!Mesh)
			return Result;
		Result->SetStringField(TEXT("skeletal_mesh"), GetPathNameSafe(Mesh->GetSkeletalMeshAsset()));
		Result->SetObjectField(TEXT("world_transform"), DescribeTransform(Mesh->GetComponentTransform()));
		Result->SetObjectField(TEXT("relative_transform"), DescribeTransform(Mesh->GetRelativeTransform()));
		UAnimInstance* Instance = Mesh->GetAnimInstance();
		Result->SetStringField(TEXT("anim_instance_class"), Instance ? Instance->GetClass()->GetPathName() : TEXT("None"));
		if (Instance)
		{
			UAnimMontage* Montage = Instance->GetCurrentActiveMontage();
			Result->SetStringField(TEXT("montage"), GetPathNameSafe(Montage));
			Result->SetNumberField(TEXT("montage_position"), Montage ? Instance->Montage_GetPosition(Montage) : -1.0);
			if (UAnimSingleNodeInstance* SingleNode = Cast<UAnimSingleNodeInstance>(Instance))
			{
				Result->SetStringField(TEXT("animation"), GetPathNameSafe(SingleNode->GetCurrentAsset()));
				Result->SetNumberField(TEXT("animation_position"), SingleNode->GetCurrentTime());
			}
		}
		TArray<FTransform> ReferencePose;
		if (bIncludeReference && Mesh->GetSkeletalMeshAsset())
		{
			const FReferenceSkeleton& ReferenceSkeleton = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
			ReferencePose = ReferenceSkeleton.GetRefBonePose();
			for (int32 Index = 0; Index < ReferencePose.Num(); ++Index)
			{
				const int32 Parent = ReferenceSkeleton.GetParentIndex(Index);
				if (Parent != INDEX_NONE)
					ReferencePose[Index] = ReferencePose[Index] * ReferencePose[Parent];
			}
		}
		TArray<TSharedPtr<FJsonValue>> ReferenceBones;
		TArray<TSharedPtr<FJsonValue>> Bones;
		const TArray<FTransform>& ComponentPose = Mesh->GetComponentSpaceTransforms();
		for (int32 Index = 0; Index < Mesh->GetNumBones(); ++Index)
		{
			const FString Name = Mesh->GetBoneName(Index).ToString();
			if (Index != 0 && !Name.Contains(TEXT("root")) && !Name.Contains(TEXT("pelvis")) && !Name.Contains(TEXT("head")) && !Name.Contains(TEXT("hips")))
				continue;
			if (!ComponentPose.IsValidIndex(Index) || Bones.Num() >= 12)
				continue;
			TSharedRef<FJsonObject> Bone = DescribeTransform(ComponentPose[Index]);
			Bone->SetStringField(TEXT("bone"), Name);
			Bone->SetNumberField(TEXT("index"), Index);
			Bones.Add(MakeShared<FJsonValueObject>(Bone));
			if (ReferencePose.IsValidIndex(Index))
			{
				TSharedRef<FJsonObject> ReferenceBone = DescribeTransform(ReferencePose[Index]);
				ReferenceBone->SetStringField(TEXT("bone"), Name);
				ReferenceBone->SetNumberField(TEXT("index"), Index);
				ReferenceBones.Add(MakeShared<FJsonValueObject>(ReferenceBone));
			}
		}
		Result->SetArrayField(TEXT("component_space_bones"), Bones);
		if (bIncludeReference)
			Result->SetArrayField(TEXT("reference_component_space_bones"), ReferenceBones);
		return Result;
	}

	TArray<TSharedPtr<FJsonValue>> DescribeFighters(UWorld* World, bool bIncludeReference = false)
	{
		TArray<TSharedPtr<FJsonValue>> Fighters;
		if (!World)
			return Fighters;
		for (TActorIterator<ASSDPawn> It(World); It; ++It)
		{
			ASSDPawn* Pawn = *It;
			TSharedRef<FJsonObject> Fighter = MakeShared<FJsonObject>();
			Fighter->SetStringField(TEXT("path"), Pawn->GetPathName());
			Fighter->SetStringField(TEXT("class"), Pawn->GetClass()->GetPathName());
			Fighter->SetStringField(TEXT("controller"), GetPathNameSafe(Pawn->GetController()));
			Fighter->SetBoolField(TEXT("hidden"), Pawn->IsHidden());
			Fighter->SetObjectField(TEXT("actor_transform"), DescribeTransform(Pawn->GetActorTransform()));
			Fighter->SetNumberField(TEXT("pawn_state"), static_cast<int32>(Pawn->PawnState));
			Fighter->SetBoolField(TEXT("performing_move"), Pawn->bIsPerformingMove);
			Fighter->SetStringField(TEXT("move_network"), GetPathNameSafe(Pawn->MoveNetwork));
			if (USSDMovementComponent* Movement = Pawn->MovementComponent)
			{
				Fighter->SetBoolField(TEXT("facing_right"), Movement->bIsFacingRight);
				Fighter->SetBoolField(TEXT("should_face_right"), Movement->bShouldFaceRight);
				Fighter->SetBoolField(TEXT("on_ground"), Movement->bOnGround);
				Fighter->SetBoolField(TEXT("can_move"), Movement->bCanMove);
				Fighter->SetNumberField(TEXT("x_speed"), Movement->XSpeed);
				Fighter->SetNumberField(TEXT("y_speed"), Movement->YSpeed);
			}
			Fighter->SetObjectField(TEXT("mesh"), DescribeMesh(Pawn->SkeletalMesh, bIncludeReference));
			Fighters.Add(MakeShared<FJsonValueObject>(Fighter));
		}
		return Fighters;
	}

	TSharedRef<FJsonObject> DescribeWorld(UWorld* World)
	{
		TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("enabled"), IsEnabled());
		Result->SetStringField(TEXT("world"), GetPathNameSafe(World));
		Result->SetNumberField(TEXT("frame"), static_cast<double>(GFrameCounter));
		if (World)
		{
			Result->SetNumberField(TEXT("time"), World->GetTimeSeconds());
			Result->SetNumberField(TEXT("delta"), World->GetDeltaSeconds());
			Result->SetBoolField(TEXT("paused"), World->IsPaused());
		}
		return Result;
	}
}

bool USSDInputProbe::StartPlay()
{
	if (!SSDInputProbe::IsEnabled() || !GEditor || SSDInputProbe::GetPlayWorld())
		return false;
	FRequestPlaySessionParams Params;
	Params.GlobalMapOverride = TEXT("/Game/Levels/MainMenu");
	Params.bAllowOnlineSubsystem = false;
	GEditor->RequestPlaySession(Params);
	return true;
}

void USSDInputProbe::EndPlay()
{
	if (SSDInputProbe::IsEnabled() && GEditor && SSDInputProbe::GetPlayWorld())
		GEditor->RequestEndPlayMap();
}

FString USSDInputProbe::SendKey(FName KeyName, bool bPressed, int32 DeviceId)
{
	UWorld* World = SSDInputProbe::GetPlayWorld();
	TSharedRef<FJsonObject> Result = SSDInputProbe::DescribeWorld(World);
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	const FKey Key(KeyName);
	if (!Viewport || !Viewport->Viewport || !Key.IsValid())
	{
		Result->SetStringField(TEXT("error"), TEXT("No active PIE viewport or invalid key"));
		return SSDInputProbe::Serialize(Result);
	}
	FInputKeyEventArgs Args(Viewport->Viewport, FInputDeviceId::CreateFromInternalId(DeviceId), Key,
		bPressed ? IE_Pressed : IE_Released, bPressed ? 1.f : 0.f, false, FPlatformTime::Cycles64());
	Result->SetStringField(TEXT("key"), Key.ToString());
	Result->SetBoolField(TEXT("pressed"), bPressed);
	Result->SetNumberField(TEXT("raw_device"), Args.InputDevice.GetId());
	Result->SetNumberField(TEXT("raw_controller_id"), Args.ControllerId);
	const FDelegateHandle Handle = Viewport->OnInputKey().AddLambda([Result, Viewport](const FInputKeyEventArgs& Routed)
	{
		Result->SetNumberField(TEXT("routed_device"), Routed.InputDevice.GetId());
		Result->SetNumberField(TEXT("routed_controller_id"), Routed.ControllerId);
		ULocalPlayer* Target = GEngine->GetLocalPlayerFromInputDevice(Viewport, Routed.InputDevice);
		Result->SetStringField(TEXT("target_player"), GetPathNameSafe(Target));
		Result->SetStringField(TEXT("target_controller"), GetPathNameSafe(Target ? Target->PlayerController : nullptr));
		Result->SetNumberField(TEXT("target_controller_id"), Target ? Target->GetControllerId() : -1);
	});
	Result->SetBoolField(TEXT("handled"), Viewport->InputKey(Args));
	Viewport->OnInputKey().Remove(Handle);
	return SSDInputProbe::Serialize(Result);
}

FString USSDInputProbe::SendAxis(FName KeyName, float Value, int32 DeviceId)
{
	UWorld* World = SSDInputProbe::GetPlayWorld();
	TSharedRef<FJsonObject> Result = SSDInputProbe::DescribeWorld(World);
	UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
	const FKey Key(KeyName);
	if (!Viewport || !Viewport->Viewport || !Key.IsAnalog())
	{
		Result->SetStringField(TEXT("error"), TEXT("No active PIE viewport or invalid analog key"));
		return SSDInputProbe::Serialize(Result);
	}
	FInputKeyEventArgs Args(Viewport->Viewport, FInputDeviceId::CreateFromInternalId(DeviceId), Key,
		Value, World->GetDeltaSeconds(), 1, FPlatformTime::Cycles64());
	Result->SetStringField(TEXT("key"), Key.ToString());
	Result->SetNumberField(TEXT("value"), Value);
	Result->SetNumberField(TEXT("raw_device"), Args.InputDevice.GetId());
	Result->SetNumberField(TEXT("raw_controller_id"), Args.ControllerId);
	const FDelegateHandle Handle = Viewport->OnInputAxis().AddLambda(
		[Result](FViewport*, int32 ControllerId, FKey, float, float, int32, bool)
		{
			Result->SetNumberField(TEXT("routed_controller_id"), ControllerId);
		});
	Result->SetBoolField(TEXT("handled"), Viewport->InputAxis(Args));
	Viewport->OnInputAxis().Remove(Handle);
	return SSDInputProbe::Serialize(Result);
}

FString USSDInputProbe::Snapshot(bool bIncludeWidgets)
{
	UWorld* World = SSDInputProbe::GetPlayWorld();
	TSharedRef<FJsonObject> Result = SSDInputProbe::DescribeWorld(World);
	if (!World)
		return SSDInputProbe::Serialize(Result);
	USSDGameInstance* GI = Cast<USSDGameInstance>(World->GetGameInstance());
	if (GI)
	{
		Result->SetNumberField(TEXT("p1_controller_id"), GI->P1ControllerID);
		Result->SetNumberField(TEXT("p2_controller_id"), GI->P2ControllerID);
		Result->SetNumberField(TEXT("menu_controller_id"), GI->MenuControllerID);
		Result->SetBoolField(TEXT("training"), GI->bTrainingMode);
		Result->SetStringField(TEXT("p1_class"), GetPathNameSafe(GI->P1Char.Get()));
		Result->SetStringField(TEXT("p2_class"), GetPathNameSafe(GI->P2Char.Get()));
		Result->SetObjectField(TEXT("game_instance_properties"), SSDInputProbe::DescribeAuthoredProperties(GI));
		TArray<TSharedPtr<FJsonValue>> Bindings;
		for (const FSSDPlayerBindings& Binding : GI->Bindings)
		{
			TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
			for (const auto& Pair : Binding.Directions)
				Entry->SetStringField(FString::Printf(TEXT("direction_%d"), static_cast<int32>(Pair.Key)), Pair.Value.ToString());
			for (const auto& Pair : Binding.Attacks)
				Entry->SetStringField(FString::Printf(TEXT("attack_%d"), static_cast<int32>(Pair.Key)), Pair.Value.ToString());
			Bindings.Add(MakeShared<FJsonValueObject>(Entry));
		}
		Result->SetArrayField(TEXT("bindings"), Bindings);
	}
	TArray<TSharedPtr<FJsonValue>> Players;
	UGameInstance* Instance = World->GetGameInstance();
	if (Instance)
		for (int32 Index = 0; Index < Instance->GetLocalPlayers().Num(); ++Index)
		{
			ULocalPlayer* Player = Instance->GetLocalPlayers()[Index];
			TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
			Entry->SetNumberField(TEXT("index"), Index);
			Entry->SetStringField(TEXT("path"), Player->GetPathName());
			Entry->SetNumberField(TEXT("controller_id"), Player->GetControllerId());
			Entry->SetNumberField(TEXT("platform_user"), Player->GetPlatformUserId().GetInternalId());
			Entry->SetNumberField(TEXT("primary_device"), IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(Player->GetPlatformUserId()).GetId());
			APlayerController* Controller = Player->PlayerController;
			if (Controller)
			{
				Entry->SetStringField(TEXT("controller"), Controller->GetPathName());
				Entry->SetStringField(TEXT("controller_class"), Controller->GetClass()->GetPathName());
				Entry->SetStringField(TEXT("pawn"), GetPathNameSafe(Controller->GetPawn()));
				Entry->SetStringField(TEXT("player_input"), GetPathNameSafe(Controller->PlayerInput));
				Entry->SetBoolField(TEXT("ignore_move"), Controller->IsMoveInputIgnored());
				Entry->SetBoolField(TEXT("ignore_look"), Controller->IsLookInputIgnored());
				Entry->SetBoolField(TEXT("right_down"), Controller->IsInputKeyDown(EKeys::Gamepad_DPad_Right));
				Entry->SetBoolField(TEXT("light_down"), Controller->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left));
				Entry->SetNumberField(TEXT("stick_x"), Controller->GetInputAnalogKeyState(EKeys::Gamepad_LeftX));
				Entry->SetNumberField(TEXT("stick_y"), Controller->GetInputAnalogKeyState(EKeys::Gamepad_LeftY));
				if (UInputComponent* Input = Controller->InputComponent)
				{
					Entry->SetNumberField(TEXT("key_bindings"), Input->KeyBindings.Num());
					Entry->SetNumberField(TEXT("action_bindings"), Input->GetNumActionBindings());
					Entry->SetBoolField(TEXT("block_input"), Input->bBlockInput);
				}
				if (ASSDPlayerController* FighterController = Cast<ASSDPlayerController>(Controller))
					if (FighterController->Inputs)
					{
						Entry->SetNumberField(TEXT("direction"), static_cast<int32>(FighterController->Inputs->DPadState));
						Entry->SetNumberField(TEXT("attack"), static_cast<int32>(FighterController->Inputs->AttackState));
						Entry->SetNumberField(TEXT("move_buffer_size"), FighterController->Inputs->MoveBuffer.Num());
					}
			}
			Players.Add(MakeShared<FJsonValueObject>(Entry));
		}
	Result->SetArrayField(TEXT("players"), Players);
	Result->SetArrayField(TEXT("fighters"), SSDInputProbe::DescribeFighters(World, true));
	if (bIncludeWidgets)
	{
		TArray<UUserWidget*> Widgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, Widgets, UUserWidget::StaticClass(), false);
		TArray<TSharedPtr<FJsonValue>> Entries;
		for (UUserWidget* Widget : Widgets)
		{
			TSharedRef<FJsonObject> Entry = MakeShared<FJsonObject>();
			Entry->SetStringField(TEXT("path"), Widget->GetPathName());
			Entry->SetStringField(TEXT("class"), Widget->GetClass()->GetPathName());
			Entry->SetBoolField(TEXT("in_viewport"), Widget->IsInViewport());
			Entry->SetBoolField(TEXT("visible"), Widget->IsVisible());
			Entry->SetBoolField(TEXT("enabled"), Widget->GetIsEnabled());
			Entry->SetObjectField(TEXT("properties"), SSDInputProbe::DescribeAuthoredProperties(Widget));
			Entries.Add(MakeShared<FJsonValueObject>(Entry));
		}
		Result->SetArrayField(TEXT("widgets"), Entries);
	}
	return SSDInputProbe::Serialize(Result);
}

FString USSDInputProbe::SnapshotFighters()
{
	UWorld* World = SSDInputProbe::GetPlayWorld();
	TSharedRef<FJsonObject> Result = SSDInputProbe::DescribeWorld(World);
	Result->SetArrayField(TEXT("fighters"), SSDInputProbe::DescribeFighters(World));
	return SSDInputProbe::Serialize(Result);
}
