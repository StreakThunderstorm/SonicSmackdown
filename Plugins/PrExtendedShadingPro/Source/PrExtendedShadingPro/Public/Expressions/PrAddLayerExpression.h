// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#pragma once

#include "PrExtendedShadingProDefines.h"

#include "CoreMinimal.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpression.h"
#include "UObject/ObjectMacros.h"

#include "PrAddLayerExpression.generated.h"

class FMaterialCompiler;
class UMaterialExpressionCustom;

UENUM()
enum class EPrMaterialExpressionTargetIndex : uint8
{
	All = 0,
	Color = 1,
	Shadow = 2,
	Auto = 3
};

UENUM()
enum class EPrMaterialExpressionLayerType : uint8
{
	Shadow = 0,
	SelfShadow = 1,
	Diffuse = 2,
	DirectionalLight = 3,
	Skylight = 4,
	Reflection = 5,
	Specular = 6,
	DynamicLight = 7,         
	DynamicLightSpecular = 8, 
	Indirect = 9,             
};

struct FPrExpressionInputDescription
{
	FPrExpressionInputDescription();
	FPrExpressionInputDescription(uint32 InType, const FString& InName, FString InTooltip, bool bInRequired, FExpressionInput* InExpressionInputPtr);

	uint32 Type;
	FString Name;
	FString Tooltip;
	bool bRequired;

	FExpressionInput* ExpressionInputPtr;
};

UCLASS()
class PREXTENDEDSHADINGPRO_API UPrAddLayerExpression : public UMaterialExpression
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	FExpressionInput Exec;
	FPrExpressionInputDescription ExecDescription;

	UPROPERTY()
	FExpressionInput Source;
	FPrExpressionInputDescription SourceDescription;

	UPROPERTY()
	FExpressionInput BlendParam0;
	FPrExpressionInputDescription BlendParam0Description;

	UPROPERTY()
	FExpressionInput BlendParam1;
	FPrExpressionInputDescription BlendParam1Description;

	UPROPERTY()
	FExpressionInput BlendParam2;
	FPrExpressionInputDescription BlendParam2Description;

	UPROPERTY()
	FExpressionInput BlendParam3;
	FPrExpressionInputDescription BlendParam3Description;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	EPrMaterialExpressionTargetIndex TargetIndex;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	EPrMaterialExpressionLayerType LayerType;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	bool bUseParam0;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	bool bUseParam1;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	bool bUseParam2;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	bool bUseParam3;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer)
	bool bLayerModify;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer, meta = (MultiLine = true))
	FString BlendCode;

	UPROPERTY(EditAnywhere, Category = MaterialExpressionAddLayer, meta = (MultiLine = true))
	FString ModifyCode;

	bool IsLayerModify() const;

	TArray<const FPrExpressionInputDescription*> GetInputsDescriptions() const;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;

	bool IsSimpleBlendCode() const;

	bool CompileInput(FMaterialCompiler* Compiler, int32 LayerIndex, UMaterialExpressionCustom* CustomExpression, FExpressionInput* ExpressionInput, FPrExpressionInputDescription* ExpressionInputDescription, const FString& StructParamName, FString& OutStructParamCode, FString& OutFunctionParamCode, TArray<int32>& OutCompiledInputs) const;

	virtual FText GetKeywords() const override;
	virtual const TArray<FExpressionInput*> GetInputs() override;
	virtual FExpressionInput* GetInput(int32 InputIndex) override;
	virtual FName GetInputName(int32 InputIndex) const override;
	virtual bool IsInputConnectionRequired(int32 InputIndex) const override;
	virtual uint32 GetInputType(int32 InputIndex) override;
	virtual uint32 GetOutputType(int32 OutputIndex) override;
	virtual FText GetCreationDescription() const override;
	virtual FText GetCreationName() const override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual void GetConnectorToolTip(int32 InputIndex, int32 OutputIndex, TArray<FString>& OutToolTip) override;
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
#endif // WITH_EDITOR

private:
	static int32 ExpressionIndex;
};
