#include "FrameDataFactory.h"
#include "FrameData.h"

#define LOCTEXT_NAMESPACE "FrameDataFactory"

UFrameDataFactory::UFrameDataFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UFrameData::StaticClass();
}

UFrameDataFactory::~UFrameDataFactory()
{

}

UObject* UFrameDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

#undef LOCTEXT_NAMESPACE
