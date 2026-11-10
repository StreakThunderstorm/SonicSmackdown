// Fill out your copyright notice in the Description page of Project Settings.

// Fill out your copyright notice in the Description page of Project Settings.

#include "SSDHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Engine.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Input/SSDPlayerController.h"

ASSDHUD::ASSDHUD()
{
	
}

void ASSDHUD::BeginPlay()
{
	Super::BeginPlay();

	
}

void ASSDHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!OwningPC)
	{
		OwningPC = Cast<ASSDPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 1));
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
			case ESSDInputDir::SSDID_None:
				ActionYOffset = 0;
				break;

			case ESSDInputDir::SSDID_Right:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.5f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_UpRight:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.25f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_Up:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.0f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_UpLeft:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.75f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_Left:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.5f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_DownLeft:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.25f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_Down:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.f, 0.f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;

			case ESSDInputDir::SSDID_DownRight:
				DrawTexture(InputDebugTexture, BorderSpacing + (i*(IconSize + IconClearance)), YPosition, IconSize, IconSize, 0.75f, 0.25f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
				break;
			}

			switch (OwningPC->InputBuffer[i + ArrayOffset].Attack)
			{
			case ESSDInputAttack::SSDIA_Light:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case ESSDInputAttack::SSDIA_Medium:
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case ESSDInputAttack::SSDIA_LightMedium:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_Heavy:
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case ESSDInputAttack::SSDIA_LightHeavy:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_MediumHeavy:
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_LightMediumHeavy:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (2 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_Special:
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				break;

			case ESSDInputAttack::SSDIA_LightSpecial:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_MediumSpecial:
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_HeavySpecial:
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				break;

			case ESSDInputAttack::SSDIA_LightMediumHeavySpecial:
				DrawLight(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset);
				DrawMedium(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (1 * (IconSize + IconClearance)));
				DrawHeavy(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (2 * (IconSize + IconClearance)));
				DrawSpecial(BorderSpacing + (i*(IconSize + IconClearance)), YPosition - ActionYOffset - (3 * (IconSize + IconClearance)));
				break;
			}

		}
	}
}

void ASSDHUD::DrawLight(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}

void ASSDHUD::DrawMedium(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.25f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}

void ASSDHUD::DrawHeavy(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.5f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}

void ASSDHUD::DrawSpecial(float XPos, float YPos)
{
	DrawTexture(InputDebugTexture, XPos, YPos, IconSize, IconSize, 0.75f, 0.5f, 0.25f, 0.25f, FLinearColor::White, EBlendMode::BLEND_Translucent, GlobalScale, true);
}



