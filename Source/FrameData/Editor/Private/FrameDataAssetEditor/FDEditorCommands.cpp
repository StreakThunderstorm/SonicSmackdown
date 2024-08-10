#include "FDEditorCommands.h"

#define LOCTEXT_NAMESPACE "EditorCommands_FrameData"

void FFDEditorCommands::RegisterCommands()
{
	UI_COMMAND(TogglePlay, "Toggle Play", "Toggle Play", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::SpaceBar));
	UI_COMMAND(StepForward, "Step Forward", "StepForward", EUserInterfaceActionType::Button, FInputChord(EKeys::X));
	UI_COMMAND(StepBack, "Step Back", "Step Back", EUserInterfaceActionType::Button, FInputChord(EKeys::Z));
}

#undef LOCTEXT_NAMESPACE
