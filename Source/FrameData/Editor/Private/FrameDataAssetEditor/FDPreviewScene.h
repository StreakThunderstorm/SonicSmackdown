#pragma once

#include "Runtime/Engine/Public/PreviewScene.h"

class FFDPreviewScene : public FPreviewScene 
{
public:

	virtual FLinearColor GetBackgroundColor() const override;

	virtual void RemoveComponent(UActorComponent* InComponent) override;
};