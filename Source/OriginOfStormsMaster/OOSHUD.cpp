// Fill out your copyright notice in the Description page of Project Settings.

// Fill out your copyright notice in the Description page of Project Settings.

#include "OOSHUD.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Input/OOSPlayerController.h"

AOOSHUD::AOOSHUD()
{
	
}

void AOOSHUD::BeginPlay()
{
	Super::BeginPlay();

	
}

void AOOSHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!OwningPC)
	{
		OwningPC = Cast<AOOSPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 1));
		if (!OwningPC) return;
		ArrayOffset = OwningPC->InputBuffer.Num() - InputsToDisplay;
		GlobalScale = Canvas->SizeX / 1920.f;
		BorderSpacing = (1920 - (InputsToDisplay * (IconSize + IconClearance))) / 2;
		YPosition = (2 * (1080 / 3)) + YOffset;
	}

	if (InputDebugTexture && InputsToDisplay)
	{
		for (int i = 0; i < InputsToDisplay; i++)
		{
			int ActionYOffset = (IconSize + IconClearance);
			switch (OwningPC->InputBuffer[i + ArrayOffset].Direction)
			{
			case EOOSInputDir::OOSID_None:
				ActionYOffset = 0;
				break;

			case EOOSInputDir::OOSID_Right:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.5f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_UpRight:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.25f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_Up:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.0f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_UpLeft:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.75f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_Left:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.5f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_DownLeft:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.25f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_Down:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case EOOSInputDir::OOSID_DownRight:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.75f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;
			}

			switch (OwningPC->InputBuffer[i + ArrayOffset].Attack)
			{
			case EOOSInputAttack::OOSIA_Light:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case EOOSInputAttack::OOSIA_Medium:
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case EOOSInputAttack::OOSIA_LightMedium:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_Heavy:
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case EOOSInputAttack::OOSIA_LightHeavy:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_MediumHeavy:
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_LightMediumHeavy:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (2 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_Special:
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case EOOSInputAttack::OOSIA_LightSpecial:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_MediumSpecial:
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_HeavySpecial:
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case EOOSInputAttack::OOSIA_LightMediumHeavySpecial:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (2 * (IconSize + IconClearance)));
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (3 * (IconSize + IconClearance)));
				break;
			}

		}
	}
}

void AOOSHUD::DrawLight(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}

void AOOSHUD::DrawMedium(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.25f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}

void AOOSHUD::DrawHeavy(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.5f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}

void AOOSHUD::DrawSpecial(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.75f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}



