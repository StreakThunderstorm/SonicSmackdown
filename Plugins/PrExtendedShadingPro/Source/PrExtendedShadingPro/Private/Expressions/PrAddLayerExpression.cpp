// Copyright 2021 Prudnikov Nikolay. All Rights Reserved.

#include "Expressions/PrAddLayerExpression.h"

#include "PrExtendedShadingPro.h"

#include "EdGraph/EdGraphNode.h"
#include "MaterialCompiler.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "MaterialExpression"

namespace
{
void PrepareCode(FString& Code)
{
	if (Code.Len() > 0)
	{
		Code.ReplaceInline(TEXT("\n"), TEXT("\r\n"), ESearchCase::CaseSensitive);
	}
}

FString PrepareFunctionCode(FString FunctionCode, FString FunctionName, FString FunctionArguments)
{
	FString Code = FString::Printf(TEXT("void %s(%s) {\n"), *FunctionName, *FunctionArguments);

	if (FunctionCode.Len() > 0)
	{
		Code += FunctionCode + TEXT("\n");
	}

	Code += TEXT("}\n");

	PrepareCode(Code);

	return Code;
}

bool IsValidPropertyType(EMaterialValueType PropertyType)
{
	switch (PropertyType)
	{
	case EMaterialValueType::MCT_Float:
	case EMaterialValueType::MCT_Float1:
	case EMaterialValueType::MCT_Float2:
	case EMaterialValueType::MCT_Float3:
	case EMaterialValueType::MCT_Float4:
		return true;
	}

	return false;
}

FString GetPropertyType(EMaterialValueType PropertyType)
{
	switch (PropertyType)
	{
	case EMaterialValueType::MCT_Float:
	case EMaterialValueType::MCT_Float1:
		return TEXT("MaterialFloat");
	case EMaterialValueType::MCT_Float2:
		return TEXT("MaterialFloat2");
	case EMaterialValueType::MCT_Float3:
		return TEXT("MaterialFloat3");
	case EMaterialValueType::MCT_Float4:
		return TEXT("MaterialFloat4");
	}

	checkNoEntry();

	return TEXT("UnknownType");
}
} // namespace

FPrExpressionInputDescription::FPrExpressionInputDescription()
	: Type(MCT_Float)
	, Name(TEXT("None"))
	, Tooltip(TEXT(""))
	, bRequired(false)
	, ExpressionInputPtr(nullptr)
{
}

FPrExpressionInputDescription::FPrExpressionInputDescription(uint32 InType, const FString& InName, FString InTooltip, bool bInRequired, FExpressionInput* InExpressionInputPtr)
	: Type(InType)
	, Name(InName)
	, Tooltip(InTooltip)
	, bRequired(bInRequired)
	, ExpressionInputPtr(InExpressionInputPtr)
{
}

int32 UPrAddLayerExpression::ExpressionIndex = 0;

UPrAddLayerExpression::UPrAddLayerExpression(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ExecDescription(MCT_Float3, "Exec", "Execution line, not used for shading", true, &Exec)
	, SourceDescription(MCT_Float3, "Source", "Layer source", false, &Source)
	, BlendParam0Description(MCT_Float4, "Param0", "Param0", true, &BlendParam0)
	, BlendParam1Description(MCT_Float4, "Param1", "Param1", true, &BlendParam1)
	, BlendParam2Description(MCT_Float4, "Param2", "Param2", true, &BlendParam2)
	, BlendParam3Description(MCT_Float4, "Param3", "Param3", true, &BlendParam3)
	, TargetIndex(EPrMaterialExpressionTargetIndex::Auto)
	, LayerType(EPrMaterialExpressionLayerType::Diffuse)
	, bUseParam0(false)
	, bUseParam1(false)
	, bUseParam2(false)
	, bUseParam3(false)
	, bLayerModify(false)
	, BlendCode(TEXT("Target += Layer * LayerModifier * Source;"))
	, ModifyCode(TEXT("Target = Layer * Source;"))
{

#if WITH_EDITORONLY_DATA
	static const FText Category_PrShading(LOCTEXT("PrShadingCategory", "PrShading"));
	MenuCategories.Add(Category_PrShading);

	bRealtimePreview = false;
	bNeedToUpdatePreview = false;
#endif
}

bool UPrAddLayerExpression::IsLayerModify() const
{
	return bLayerModify ||
		   LayerType == EPrMaterialExpressionLayerType::SelfShadow ||
		   LayerType == EPrMaterialExpressionLayerType::Shadow;
}

TArray<const FPrExpressionInputDescription*> UPrAddLayerExpression::GetInputsDescriptions() const
{
	TArray<const FPrExpressionInputDescription*> Result;

	Result.Add(&ExecDescription);
	Result.Add(&SourceDescription);

	if (bUseParam0)
	{
		Result.Add(&BlendParam0Description);
	}

	if (bUseParam1)
	{
		Result.Add(&BlendParam1Description);
	}

	if (bUseParam2)
	{
		Result.Add(&BlendParam2Description);
	}

	if (bUseParam3)
	{
		Result.Add(&BlendParam3Description);
	}

	return Result;
}

#if WITH_EDITOR
void UPrAddLayerExpression::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, bUseParam0) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, bUseParam1) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, bUseParam2) ||
		PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, bUseParam3))
	{
		if (GraphNode)
		{
			GraphNode->ReconstructNode();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool UPrAddLayerExpression::CanEditChange(const FProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);
	if (bIsEditable && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, TargetIndex))
		{
			bIsEditable = !IsLayerModify();
		}

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, BlendCode))
		{
			bIsEditable = !IsLayerModify();
		}

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPrAddLayerExpression, ModifyCode))
		{
			bIsEditable = IsLayerModify();
		}
	}

	return bIsEditable;
}

FText UPrAddLayerExpression::GetKeywords() const
{
	return FText::FromString(TEXT("Layer"));
}

const TArray<FExpressionInput*> UPrAddLayerExpression::GetInputs()
{
	TArray<FExpressionInput*> Result;

	for (auto DescriptionPtr : GetInputsDescriptions())
	{
		Result.Add(DescriptionPtr->ExpressionInputPtr);
	}

	return Result;
}

FExpressionInput* UPrAddLayerExpression::GetInput(int32 InputIndex)
{
	auto Inputs = GetInputsDescriptions();

	if (Inputs.IsValidIndex(InputIndex))
	{
		return Inputs[InputIndex]->ExpressionInputPtr;
	}

	return nullptr;
}

FName UPrAddLayerExpression::GetInputName(int32 InputIndex) const
{
	auto Inputs = GetInputsDescriptions();

	if (Inputs.IsValidIndex(InputIndex))
	{
		return *Inputs[InputIndex]->Name;
	}

	return NAME_None;
}

bool UPrAddLayerExpression::IsInputConnectionRequired(int32 InputIndex) const
{
	auto Inputs = GetInputsDescriptions();

	if (Inputs.IsValidIndex(InputIndex))
	{
		return Inputs[InputIndex]->bRequired;
	}

	return false;
}

uint32 UPrAddLayerExpression::GetInputType(int32 InputIndex)
{
	auto Inputs = GetInputsDescriptions();

	if (Inputs.IsValidIndex(InputIndex))
	{
		return Inputs[InputIndex]->Type;
	}

	return MCT_Float;
}

uint32 UPrAddLayerExpression::GetOutputType(int32 OutputIndex)
{
	return GetInputType(0);
}

FText UPrAddLayerExpression::GetCreationDescription() const
{
	return LOCTEXT("PrShadingLayerDescription", "Create special node for extended shading.");
}

FText UPrAddLayerExpression::GetCreationName() const
{
	return LOCTEXT("PrShadingLayer", "PrLayer");
}

void UPrAddLayerExpression::GetCaption(TArray<FString>& OutCaptions) const
{
	UEnum* TargetIndexEnum = StaticEnum<EPrMaterialExpressionTargetIndex>();
	UEnum* LayerTypeEnum = StaticEnum<EPrMaterialExpressionLayerType>();

	check(TargetIndexEnum && LayerTypeEnum);

	auto TargetIndexName = TargetIndexEnum->GetNameStringByValue(static_cast<int64>(TargetIndex));
	auto LayerTypeName = LayerTypeEnum->GetNameStringByValue(static_cast<int64>(LayerType));

	if (IsLayerModify())
	{
		OutCaptions.Add(FString::Printf(TEXT("PrLayer: Modify %s Layer"), *LayerTypeName));
	}
	else
	{
		if (TargetIndex == EPrMaterialExpressionTargetIndex::Auto)
		{
			OutCaptions.Add(FString::Printf(TEXT("PrLayer: Apply %s Layer"), *LayerTypeName));
		}
		else
		{
			OutCaptions.Add(FString::Printf(TEXT("PrLayer: Apply %s Layer for %s"), *LayerTypeName, *TargetIndexName));
		}
	}
}

void UPrAddLayerExpression::GetConnectorToolTip(int32 InputIndex, int32 OutputIndex, TArray<FString>& OutToolTip)
{
	if (InputIndex != INDEX_NONE)
	{
		auto Inputs = GetInputsDescriptions();

		if (Inputs.IsValidIndex(InputIndex))
		{
			OutToolTip.Add(Inputs[InputIndex]->Tooltip);
		}
	}
}

bool UPrAddLayerExpression::IsSimpleBlendCode() const
{
	auto Code = BlendCode.TrimStartAndEnd();
	int32 Index = INDEX_NONE;
	if (!Code.FindChar('\n', Index))
	{
		Code.ReplaceInline(TEXT(" "), TEXT(""), ESearchCase::CaseSensitive);
		const FString Target = TEXT("Target");
		if (Code.StartsWith(Target + TEXT("+="), ESearchCase::CaseSensitive))
		{
			return Code.Find(Target, ESearchCase::CaseSensitive, ESearchDir::FromStart, Target.Len()) == INDEX_NONE;
		}
	}
	return false;
}

bool UPrAddLayerExpression::CompileInput(FMaterialCompiler* Compiler, int32 LayerIndex, UMaterialExpressionCustom* CustomExpression, FExpressionInput* ExpressionInput, FPrExpressionInputDescription* ExpressionInputDescription, const FString& StructParamName, FString& OutStructParamCode, FString& OutFunctionParamCode, TArray<int32>& OutCompiledInputs) const
{
	const auto ExpressionInputIndex = ExpressionInput->GetTracedInput().Expression ? ExpressionInput->Compile(Compiler) : Compiler->Constant4(1.f, 1.f, 1.f, 1.f);
	auto ExpressionInputType = Compiler->GetType(ExpressionInputIndex);
	if (IsValidPropertyType(ExpressionInputType))
	{
		const auto StructPropertyType = GetPropertyType(ExpressionInputType);
		const auto StructPropertyName = FString::Printf(TEXT("%s.%s"), *StructParamName, *ExpressionInputDescription->Name);

		CustomExpression->Code += FString::Printf(TEXT("\t%s = %s;\n"), *StructPropertyName, *ExpressionInputDescription->Name);
		OutStructParamCode += FString::Printf(TEXT("\t%s %s;\n"), *StructPropertyType, *ExpressionInputDescription->Name);
		OutFunctionParamCode += FString::Printf(TEXT("\t%s %s = %s;\n"), *StructPropertyType, *ExpressionInputDescription->Name, *StructPropertyName);

		CustomExpression->Inputs.Add({*ExpressionInputDescription->Name, {}});
		OutCompiledInputs.Add(ExpressionInputIndex);

		return true;
	}
	else
	{
		Compiler->Errorf(TEXT("Bad type for %s input"), *ExpressionInputDescription->Name);
		return false;
	}
}

int32 UPrAddLayerExpression::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	for (auto DescriptionPtr : GetInputsDescriptions())
	{
		if (DescriptionPtr->bRequired && !DescriptionPtr->ExpressionInputPtr->GetTracedInput().Expression)
		{
			return Compiler->Errorf(TEXT("Missing %s input"), *DescriptionPtr->Name);
		}
	}

	UEnum* LayerTypeEnum = StaticEnum<EPrMaterialExpressionLayerType>();
	if (!LayerTypeEnum || !LayerTypeEnum->IsValidEnumValue(static_cast<int64>(LayerType)))
	{
		return Compiler->Errorf(TEXT("Unknown layer type"));
	}

	const int32 ExecIndex = Exec.Compile(Compiler);

	const auto LayerIndex = ++ExpressionIndex; //TODO: Use constant index for this expression for each compiler

	const auto LayerTypeName = LayerTypeEnum->GetNameStringByValue(static_cast<int64>(LayerType));
	const auto BlendCodeFunctionName = FString::Printf(TEXT("PrBlend%sLayerFunction%d"), *LayerTypeName, LayerIndex);
	const auto WBlendCodeFunctionName = FString::Printf(TEXT("PrWBlend%sLayerFunction%d"), *LayerTypeName, LayerIndex);
	const auto BlendCodeStructParamName = FString::Printf(TEXT("PrBlend%sLayerParams%d"), *LayerTypeName, LayerIndex);

	FString StructParamCode = FString::Printf(TEXT("struct F%s {\n"), *BlendCodeStructParamName);
	FString FunctionParamCode;
	TArray<int32> CompiledInputs;

	UMaterialExpressionCustom* CustomExpression = NewObject<UMaterialExpressionCustom>();
	CustomExpression->Inputs.Reset();
	CustomExpression->Code.Reset();

	if (!CompileInput(Compiler, LayerIndex, CustomExpression, &Source, &SourceDescription, BlendCodeStructParamName, StructParamCode, FunctionParamCode, CompiledInputs))
	{
		return INDEX_NONE;
	}

	if (bUseParam0)
	{
		if (!CompileInput(Compiler, LayerIndex, CustomExpression, &BlendParam0, &BlendParam0Description, BlendCodeStructParamName, StructParamCode, FunctionParamCode, CompiledInputs))
		{
			return INDEX_NONE;
		}
	}

	if (bUseParam1)
	{
		if (!CompileInput(Compiler, LayerIndex, CustomExpression, &BlendParam1, &BlendParam1Description, BlendCodeStructParamName, StructParamCode, FunctionParamCode, CompiledInputs))
		{
			return INDEX_NONE;
		}
	}

	if (bUseParam2)
	{
		if (!CompileInput(Compiler, LayerIndex, CustomExpression, &BlendParam2, &BlendParam2Description, BlendCodeStructParamName, StructParamCode, FunctionParamCode, CompiledInputs))
		{
			return INDEX_NONE;
		}
	}

	if (bUseParam3)
	{
		if (!CompileInput(Compiler, LayerIndex, CustomExpression, &BlendParam3, &BlendParam3Description, BlendCodeStructParamName, StructParamCode, FunctionParamCode, CompiledInputs))
		{
			return INDEX_NONE;
		}
	}

	CustomExpression->Code += FString::Printf(TEXT("\treturn 1.0;"));
	StructParamCode += FString::Printf(TEXT("};\n"));
	StructParamCode += FString::Printf(TEXT("static F%s %s;\n"), *BlendCodeStructParamName, *BlendCodeStructParamName);
	PrepareCode(StructParamCode);

	FString WBlendCode = FString::Printf(TEXT("\tif (Values.%sLayer.bEnabled) {\n"), *LayerTypeName);
	if (IsLayerModify())
	{
		WBlendCode += FString::Printf(TEXT("\t\t%s(Values.%sLayer.ModifiedColor, Values.%sLayer.Color, Values.%sLayer.LayerModifier);\n"), *BlendCodeFunctionName, *LayerTypeName, *LayerTypeName, *LayerTypeName);
	}
	else
	{
		EPrMaterialExpressionTargetIndex ResultTargetIndex = TargetIndex;
		if (TargetIndex == EPrMaterialExpressionTargetIndex::Auto)
		{
			switch (LayerType)
			{
			case EPrMaterialExpressionLayerType::Shadow:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::SelfShadow:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::Diffuse:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::DirectionalLight:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::Color;
				break;
			case EPrMaterialExpressionLayerType::Skylight:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::Reflection:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::Specular:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::Color;
				break;

			case EPrMaterialExpressionLayerType::DynamicLight:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::DynamicLightSpecular:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;
			case EPrMaterialExpressionLayerType::Indirect:
				ResultTargetIndex = EPrMaterialExpressionTargetIndex::All;
				break;

			default:
				break;
			}
		}

		if (ResultTargetIndex == EPrMaterialExpressionTargetIndex::All && IsSimpleBlendCode())
		{
			WBlendCode += FString::Printf(TEXT("\t\tMaterialFloat3 Target = 0.0;\n"));
			WBlendCode += FString::Printf(TEXT("\t\t%s(Target, Values.%sLayer.ModifiedColor, Values.%sLayer.LayerModifier);\n"), *BlendCodeFunctionName, *LayerTypeName, *LayerTypeName);
			WBlendCode += FString::Printf(TEXT("\t\tAccumulatedColor += Target;\n"));
			WBlendCode += FString::Printf(TEXT("\t\tAccumulatedShadow += Target;\n"));
		}
		else
		{
			if (ResultTargetIndex == EPrMaterialExpressionTargetIndex::Color || ResultTargetIndex == EPrMaterialExpressionTargetIndex::All)
			{
				WBlendCode += FString::Printf(TEXT("\t\t%s(AccumulatedColor, Values.%sLayer.ModifiedColor, Values.%sLayer.LayerModifier);\n"), *BlendCodeFunctionName, *LayerTypeName, *LayerTypeName);
			}

			if (ResultTargetIndex == EPrMaterialExpressionTargetIndex::Shadow || ResultTargetIndex == EPrMaterialExpressionTargetIndex::All)
			{
				WBlendCode += FString::Printf(TEXT("\t\t%s(AccumulatedShadow, Values.%sLayer.ModifiedColor, Values.%sLayer.LayerModifier);\n"), *BlendCodeFunctionName, *LayerTypeName, *LayerTypeName);
			}
		}
	}
	WBlendCode += TEXT("\t}");

	FString DefineCode = TEXT("1\n");

	DefineCode += TEXT("#ifndef PR_SHADER\n");
	DefineCode += TEXT("#define PR_SHADER 1\n");
	DefineCode += TEXT("#endif\n");

	DefineCode += TEXT("#ifndef PR_SHADER_PRO\n");
	DefineCode += FString::Printf(TEXT("#define PR_SHADER_PRO %d\n"), PR_SHADER_PRO);
	DefineCode += TEXT("#endif\n");

	if (LayerType == EPrMaterialExpressionLayerType::Skylight)
	{
		DefineCode += TEXT("#ifndef PR_SHADER_SL_INDIRECT\n");
		DefineCode += TEXT("#define PR_SHADER_SL_INDIRECT 1\n");
		DefineCode += TEXT("#endif\n");
	}


	if (LayerType == EPrMaterialExpressionLayerType::Indirect)
	{
		DefineCode += TEXT("#ifndef PR_SHADER_LM_INDIRECT\n");
		DefineCode += TEXT("#define PR_SHADER_LM_INDIRECT 1\n");
		DefineCode += TEXT("#endif\n");
	}


	DefineCode += TEXT("#include \"/Plugin/PrExtendedShadingPro/Private/PrudnikovShaderTypes.ush\"\n");

	auto Code = IsLayerModify() ? ModifyCode : BlendCode;

	DefineCode += StructParamCode;
	DefineCode += PrepareFunctionCode(FunctionParamCode + Code.TrimStartAndEnd(), BlendCodeFunctionName, TEXT("inout MaterialFloat3 Target, MaterialFloat3 Layer, MaterialFloat3 LayerModifier"));
	DefineCode += PrepareFunctionCode(WBlendCode, WBlendCodeFunctionName, TEXT("inout MaterialFloat3 AccumulatedColor, inout MaterialFloat3 AccumulatedShadow, inout FPrShadingValues Values"));

	DefineCode += FString::Printf(TEXT("#ifdef PR_SHADER_BLEND_LAYER_FUNCTION_%d\n"), LayerIndex - 1);
	DefineCode += FString::Printf(TEXT("\t#define PR_SHADER_BLEND_LAYER_FUNCTION_%d PR_SHADER_BLEND_LAYER_FUNCTION_%d %s(AccumulatedColor, AccumulatedShadow, Values);\n"), LayerIndex, LayerIndex - 1, *WBlendCodeFunctionName);
	DefineCode += FString::Printf(TEXT("#else\n"));
	DefineCode += FString::Printf(TEXT("\t#define PR_SHADER_BLEND_LAYER_FUNCTION_%d %s(AccumulatedColor, AccumulatedShadow, Values);\n"), LayerIndex, *WBlendCodeFunctionName);
	DefineCode += FString::Printf(TEXT("#endif\n"));

	DefineCode += FString::Printf(TEXT("#ifdef PR_SHADER_BLEND_FUNCTION\n"));
	DefineCode += FString::Printf(TEXT("\t#undef PR_SHADER_BLEND_FUNCTION\n"));
	DefineCode += FString::Printf(TEXT("#endif\n"));
	DefineCode += FString::Printf(TEXT("#define PR_SHADER_BLEND_FUNCTION PR_SHADER_BLEND_LAYER_FUNCTION_%d\n"), LayerIndex);


	DefineCode += FString::Printf(TEXT("#ifdef PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d\n"), LayerIndex - 1);
	if (LayerType == EPrMaterialExpressionLayerType::Shadow)
	{
		DefineCode += FString::Printf(TEXT("\t#define PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d %s(_ShadowTarget, _ShadowTarget, 1.0);\n"), LayerIndex, LayerIndex - 1, *BlendCodeFunctionName);
		DefineCode += FString::Printf(TEXT("#else\n"));
		DefineCode += FString::Printf(TEXT("\t#define PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d %s(_ShadowTarget, _ShadowTarget, 1.0);\n"), LayerIndex, *BlendCodeFunctionName);
	}
	else
	{
		DefineCode += FString::Printf(TEXT("\t#define PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d\n"), LayerIndex, LayerIndex - 1);
		DefineCode += FString::Printf(TEXT("#else\n"));
		DefineCode += FString::Printf(TEXT("\t#define PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d \n"), LayerIndex);
	}
	DefineCode += FString::Printf(TEXT("#endif\n"));

	DefineCode += FString::Printf(TEXT("#ifdef PR_SHADER_SHADOW_BLEND_FUNCTION\n"));
	DefineCode += FString::Printf(TEXT("\t#undef PR_SHADER_SHADOW_BLEND_FUNCTION\n"));
	DefineCode += FString::Printf(TEXT("#endif\n"));
	DefineCode += FString::Printf(TEXT("#define PR_SHADER_SHADOW_BLEND_FUNCTION(__NAME__) half3 _ShadowTarget = __NAME__; PR_SHADER_SHADOW_BLEND_LAYER_FUNCTION_%d __NAME__ = _ShadowTarget.r;\n"), LayerIndex);


	PrepareCode(DefineCode);

	CustomExpression->AdditionalDefines.Add({FString::Printf(TEXT("PR_SHADER_LAYER_%d"), LayerIndex), DefineCode});

#if ENGINE_MINOR_VERSION >= 26
	Compiler->CustomExpression(CustomExpression, OutputIndex, CompiledInputs);
#else
	Compiler->CustomExpression(CustomExpression, CompiledInputs);
#endif

	return ExecIndex;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE