#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MoveNetworkFactory.generated.h"

UCLASS()
class MOVENETWORKEDITOR_API UMoveNetworkFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMoveNetworkFactory();
	virtual ~UMoveNetworkFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
