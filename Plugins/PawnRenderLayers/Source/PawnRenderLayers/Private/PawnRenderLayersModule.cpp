#include "Modules/ModuleManager.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Runtime/Launch/Resources/Version.h"
#include "ShaderCore.h"

DEFINE_LOG_CATEGORY_STATIC(LogPawnRenderLayers, Log, All);

namespace PawnRenderLayerShaders
{
    const FString Include = TEXT("#include \"/Plugin/PawnRenderLayers/Private/PawnRenderLayers.ush\"\n");

    FString Apply(const TCHAR* Position, bool bPrevious = false)
    {
        const TCHAR* Matrix = bPrevious ? TEXT("PrevViewToClip") : TEXT("ViewToClip");
        return FString::Printf(TEXT("ApplyPawnRenderLayer(%s, GetPrimitiveData(VertexParameters).CustomPrimitiveData[7], "
            "GetPrimitiveData(VertexParameters).CustomPrimitiveData[8], "
            "float3(ResolvedView.%s[2][2], ResolvedView.%s[3][2], ResolvedView.%s[3][3]))"),
            Position, Matrix, Matrix, Matrix);
    }

    bool SaveChanged(const FString& Filename, const FString& Text)
    {
        FString Existing;
        if (FFileHelper::LoadFileToString(Existing, *Filename) && Existing == Text)
            return true;
        return FFileHelper::SaveStringToFile(Text, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    }

    bool MirrorDirectory(const FString& Source, const FString& Destination)
    {
        TArray<FString> Files;
        IFileManager::Get().FindFilesRecursive(Files, *Source, TEXT("*"), true, false);
        for (const FString& File : Files)
        {
            FString Relative = File;
            FPaths::MakePathRelativeTo(Relative, *(Source / TEXT("")));
            const FString Target = Destination / Relative;
            if (!IFileManager::Get().MakeDirectory(*FPaths::GetPath(Target), true))
                return false;
            FString Content;
            if (!FFileHelper::LoadFileToString(Content, *File) || !SaveChanged(Target, Content))
                return false;
        }
        return !Files.IsEmpty();
    }

    bool ReplaceOnce(FString& Text, const FString& From, const FString& To)
    {
        const int32 Index = Text.Find(From, ESearchCase::CaseSensitive);
        if (Index == INDEX_NONE || Text.Find(From, ESearchCase::CaseSensitive, ESearchDir::FromStart, Index + From.Len()) != INDEX_NONE)
            return false;
        Text = Text.Left(Index) + To + Text.Mid(Index + From.Len());
        return true;
    }

    bool PatchVertexShader(const FString& Filename, bool bBasePass, bool bPrudnikov)
    {
        FString Text;
        if (!FFileHelper::LoadFileToString(Text, *Filename))
            return false;
        Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
        const FString Anchor = bPrudnikov ? TEXT("Output.Position.z += GetPrimitiveData(VertexParameters).CustomPrimitiveData[0].a;")
            : bBasePass ? TEXT("Output.Position = INVARIANT(ClipSpacePosition);")
            : TEXT("Output.Position = INVARIANT(mul(RasterizedWorldPosition, ResolvedView.TranslatedWorldToClip));");
        FString Replacement = Anchor + TEXT("\n\t\tOutput.Position = INVARIANT(") + Apply(TEXT("Output.Position")) + TEXT(");");
        if (bBasePass)
            Replacement += TEXT("\n\t\tClipSpacePosition = Output.Position;");
        if (!ReplaceOnce(Text, Anchor, Replacement))
            return false;
        if (bBasePass)
        {
            const FString Previous = TEXT("PrevTranslatedWorldPosition = mul(float4(PrevTranslatedWorldPosition.xyz, 1), ")
                TEXT("ResolvedView.PrevTranslatedWorldToClip);");
            const FString PreviousReplacement = Previous + TEXT("\n\t\t\tPrevTranslatedWorldPosition = ")
                + Apply(TEXT("PrevTranslatedWorldPosition"), true) + TEXT(";");
            if (!ReplaceOnce(Text, Previous, PreviousReplacement))
                return false;
        }
        return SaveChanged(Filename, Include + Text);
    }

    bool PatchVelocityShader(const FString& Filename)
    {
        FString Text;
        if (!FFileHelper::LoadFileToString(Text, *Filename))
            return false;
        Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
        const FString CurrentReplacement = TEXT("ScreenPos = ") + Apply(TEXT("ScreenPos")) + TEXT(";\n\t\t\tOutput.Position = ScreenPos;");
        if (!ReplaceOnce(Text, TEXT("Output.Position = ScreenPos;"), CurrentReplacement))
            return false;
        const FString Previous = TEXT("float4 PrevScreenPosObj = mul(float4(PrevTranslatedWorldPosition.xyz, 1), ")
            TEXT("ResolvedView.PrevTranslatedWorldToClip);");
        if (!ReplaceOnce(Text, Previous, Previous + TEXT("\n\t\tPrevScreenPosObj = ") + Apply(TEXT("PrevScreenPosObj"), true) + TEXT(";")))
            return false;
        return SaveChanged(Filename, Include + Text);
    }

    bool PatchPositionOnlyShader(const FString& Filename)
    {
        FString Text;
        if (!FFileHelper::LoadFileToString(Text, *Filename))
            return false;
        Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
        const FString Anchor = TEXT("OutPosition = INVARIANT(mul(WorldPos, ResolvedView.TranslatedWorldToClip));");
        FString Function = Apply(TEXT("OutPosition"));
        Function.ReplaceInline(TEXT("GetPrimitiveData(VertexParameters)"), TEXT("VF_GPUSCENE_GET_INTERMEDIATES(Input).Primitive"));
        if (!ReplaceOnce(Text, Anchor, Anchor + TEXT("\n\t\tOutPosition = INVARIANT(") + Function + TEXT(");")))
            return false;
        return SaveChanged(Filename, Include + Text);
    }
}

class FPawnRenderLayersModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
#if WITH_EDITOR
        if (ENGINE_MAJOR_VERSION != 5 || ENGINE_MINOR_VERSION != 8 || ENGINE_PATCH_VERSION != 4)
        {
            UE_LOG(LogPawnRenderLayers, Error, TEXT("Shader overlay supports UE 5.8.4 only. No shader mappings changed."));
            return;
        }
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("PawnRenderLayers"));
        const TSharedPtr<IPlugin> ShadingPlugin = IPluginManager::Get().FindPlugin(TEXT("PrExtendedShadingPro"));
        if (!Plugin || !ShadingPlugin)
            return;

        const FString Overlay = Plugin->GetBaseDir() / TEXT("Intermediate/ShaderOverlay/5.8.4");
        const FString EngineOverlay = Overlay / TEXT("EnginePrivate");
        const FString ShadingOverlay = Overlay / TEXT("PrExtendedShadingPro");
        const FString ShadingSource = ShadingPlugin->GetBaseDir() / TEXT("Shaders/Private");
        using namespace PawnRenderLayerShaders;
        bool bSuccess = MirrorDirectory(FPaths::EngineDir() / TEXT("Shaders/Private"), EngineOverlay);
        bSuccess = bSuccess && MirrorDirectory(ShadingSource, ShadingOverlay);
        bSuccess = bSuccess && PatchVertexShader(EngineOverlay / TEXT("BasePassVertexShader.usf"), true, false);
        bSuccess = bSuccess && PatchVertexShader(EngineOverlay / TEXT("DepthOnlyVertexShader.usf"), false, false);
        bSuccess = bSuccess && PatchPositionOnlyShader(EngineOverlay / TEXT("PositionOnlyDepthVertexShader.usf"));
        bSuccess = bSuccess && PatchVelocityShader(EngineOverlay / TEXT("VelocityShader.usf"));
        // The project's PrExtendedShadingPro port selects its 5.8.3 implementation on UE 5.8.4.
        bSuccess = bSuccess && PatchVertexShader(ShadingOverlay / TEXT("5.8.3/BasePassVertexShader.usf"), true, true);
        bSuccess = bSuccess && PatchVertexShader(ShadingOverlay / TEXT("5.8.3/DepthOnlyVertexShader.usf"), false, true);
        if (!bSuccess)
        {
            UE_LOG(LogPawnRenderLayers, Error, TEXT("Shader overlay preparation failed; no shader mappings changed."));
            return;
        }

        // More-specific virtual directories shadow parent mappings without overwriting engine or plugin sources.
        const FString EngineVirtual = TEXT("/Engine/Private");
        const FString ShadingVirtual = TEXT("/Plugin/PrExtendedShadingPro/Private");
        const auto& Mappings = AllShaderSourceDirectoryMappings();
        if (Mappings.Contains(EngineVirtual) || Mappings.Contains(ShadingVirtual))
        {
            UE_LOG(LogPawnRenderLayers, Error, TEXT("Another plugin owns the shader overlay directories; refusing to replace its mappings."));
            return;
        }
        AddShaderSourceDirectoryMapping(TEXT("/Plugin/PawnRenderLayers"), Plugin->GetBaseDir() / TEXT("Shaders"));
        AddShaderSourceDirectoryMapping(EngineVirtual, EngineOverlay);
        AddShaderSourceDirectoryMapping(ShadingVirtual, ShadingOverlay);
        UE_LOG(LogPawnRenderLayers, Display, TEXT("UE 5.8.4 pawn layer shader overlays enabled. Engine and existing plugin files are untouched."));
#endif
    }
};

IMPLEMENT_MODULE(FPawnRenderLayersModule, PawnRenderLayers)
