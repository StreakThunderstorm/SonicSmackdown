
#include "FDPreviewScene.h"

FLinearColor FFDPreviewScene::GetBackgroundColor() const
{
	FLinearColor BackgroundColor = FColor(128, 128, 128);
	return BackgroundColor;
}

void FFDPreviewScene::RemoveComponent(UActorComponent* InComponent)
{
	if (!InComponent->IsValidLowLevel()) return;

	FPreviewScene::RemoveComponent(InComponent);
}
