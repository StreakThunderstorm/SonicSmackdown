#include "MoveNetworkFactory.h"
#include "MoveNetwork.h"

#define LOCTEXT_NAMESPACE "MoveNetworkFactory"

UMoveNetworkFactory::UMoveNetworkFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UMoveNetwork::StaticClass();
}

UMoveNetworkFactory::~UMoveNetworkFactory()
{

}

UObject* UMoveNetworkFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE
