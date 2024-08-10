#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "FrameDataFactory.generated.h"

UCLASS()
class FRAMEDATAEDITOR_API UFrameDataFactory : public UFactory
{
	GENERATED_BODY()

public:
	UFrameDataFactory();
	virtual ~UFrameDataFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
