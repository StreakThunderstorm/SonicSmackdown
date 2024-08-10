#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FDTrigger.generated.h"

UCLASS(Abstract)
class FRAMEDATARUNTIME_API UFDTrigger : public UObject
{
	GENERATED_BODY()

public:
	UFDTrigger();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger | Basic", meta = (DisplayPriority = "1"))
	int Duration;

	// Must be implemented, marking base class as abstract so it doesn't show up on the add trigger menu.
	virtual void TriggerStart() {}
	virtual void TriggerTick() {}
	virtual void TriggerEnd() {}

#if WITH_EDITORONLY_DATA
	FLinearColor EditorColor;
#endif
};
