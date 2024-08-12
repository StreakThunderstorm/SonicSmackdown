// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SFDTriggerPanel.h"
#include "FrameData.h"
#include "FDTrigger.h"
#include "Engine.h"
#include "Editor\ClassViewer\Public\ClassViewerModule.h"
#include "Editor\ClassViewer\Public\ClassViewerFilter.h"
#include "Runtime\CoreUObject\Public\UObject\UObjectIterator.h"
#include "Runtime\Slate\Public\Framework\Application\SlateApplication.h"
#include "Runtime\Slate\Public\Framework\MultiBox\MultiBoxBuilder.h"
#include "Runtime\SlateCore\Public\Layout\WidgetPath.h"
#include "IDetailsView.h"
#include "IStructureDetailsView.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FDTriggerPanel"

/**
 * Slate button that replaces OnClicked delegate with OnClicked_WithPos to send mouse pointer position.
 * Used to spawn a trigger menu at the click location in the FrameData editor.
 */
class FRAMEDATAEDITOR_API SFDButton : public SButton
{
public:

	DECLARE_DELEGATE_RetVal_OneParam(FReply, FOnClicked_WithPos, FVector2D)

public:

	SLATE_BEGIN_ARGS(SFDButton)
		: _Content()
		, _ButtonStyle(&FCoreStyle::Get().GetWidgetStyle< FButtonStyle >("Button"))
		, _TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText"))
		, _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _ContentPadding(FMargin(4.0, 2.0))
		, _Text()
		, _ClickMethod(EButtonClickMethod::DownAndUp)
		, _TouchMethod(EButtonTouchMethod::DownAndUp)
		, _PressMethod(EButtonPressMethod::DownAndUp)
		, _DesiredSizeScale(FVector2D(1, 1))
		, _ContentScale(FVector2D(1, 1))
		, _ButtonColorAndOpacity(FLinearColor::White)
		, _ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
		, _IsFocusable(true)
	{
	}

	/** Slot for this button's content (optional) */
	SLATE_DEFAULT_SLOT(FArguments, Content)

		/** The visual style of the button */
		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)

		/** The text style of the button */
		SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)

		/** Horizontal alignment */
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)

		/** Vertical alignment */
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)

		/** Spacing between button's border and the content. */
		SLATE_ATTRIBUTE(FMargin, ContentPadding)

		/** The text to display in this button, if no custom content is specified */
		SLATE_ATTRIBUTE(FText, Text)

		/** Called when the button is clicked, sends mouse location */
		SLATE_EVENT(FOnClicked_WithPos, OnClicked)

		/** Called when the button is pressed */
		SLATE_EVENT(FSimpleDelegate, OnPressed)

		/** Called when the button is released */
		SLATE_EVENT(FSimpleDelegate, OnReleased)

		SLATE_EVENT(FSimpleDelegate, OnHovered)

		SLATE_EVENT(FSimpleDelegate, OnUnhovered)

		/** Sets the rules to use for determining whether the button was clicked.  This is an advanced setting and generally should be left as the default. */
		SLATE_ARGUMENT(EButtonClickMethod::Type, ClickMethod)

		/** How should the button be clicked with touch events? */
		SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)

		/** How should the button be clicked with keyboard/controller button events? */
		SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)

		SLATE_ATTRIBUTE(FVector2D, DesiredSizeScale)

		SLATE_ATTRIBUTE(FVector2D, ContentScale)

		SLATE_ATTRIBUTE(FSlateColor, ButtonColorAndOpacity)

		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)

		/** Sometimes a button should only be mouse-clickable and never keyboard focusable. */
		SLATE_ARGUMENT(bool, IsFocusable)

		/** The sound to play when the button is pressed */
		SLATE_ARGUMENT(TOptional<FSlateSound>, PressedSoundOverride)

		/** The sound to play when the button is hovered */
		SLATE_ARGUMENT(TOptional<FSlateSound>, HoveredSoundOverride)

		/** Which text shaping method should we use? (unset to use the default returned by GetDefaultTextShapingMethod) */
		SLATE_ARGUMENT(TOptional<ETextShapingMethod>, TextShapingMethod)

		/** Which text flow direction should we use? (unset to use the default returned by GetDefaultTextFlowDirection) */
		SLATE_ARGUMENT(TOptional<ETextFlowDirection>, TextFlowDirection)


	SLATE_END_ARGS()

	/**
	* Construct this widget
	*
	* @param	InArgs	The declaration data for this widget
	*/
	void Construct(const FArguments& InArgs);

private:

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/* Event OnClicked delegate*/
	FOnClicked_WithPos OnClicked_WithPos;

};

void SFDButton::Construct(const FArguments& InArgs)
{
	SButton::Construct
	(
		SButton::FArguments()
		.ButtonStyle(InArgs._ButtonStyle)
		.TextStyle(InArgs._TextStyle)
		.HAlign(InArgs._HAlign)
		.VAlign(InArgs._VAlign)
		.ContentPadding(InArgs._ContentPadding)
		.Text(InArgs._Text)
		.OnPressed(InArgs._OnPressed)
		.OnReleased(InArgs._OnReleased)
		.OnHovered(InArgs._OnHovered)
		.OnUnhovered(InArgs._OnUnhovered)
		.ClickMethod(InArgs._ClickMethod)
		.TouchMethod(InArgs._TouchMethod)
		.PressMethod(InArgs._PressMethod)
		.DesiredSizeScale(InArgs._DesiredSizeScale)
		.ContentScale(InArgs._ContentScale)
		.ButtonColorAndOpacity(InArgs._ButtonColorAndOpacity)
		.ForegroundColor(InArgs._ForegroundColor)
		.IsFocusable(InArgs._IsFocusable)
		.PressedSoundOverride(InArgs._PressedSoundOverride)
		.HoveredSoundOverride(InArgs._HoveredSoundOverride)
		.TextShapingMethod(InArgs._TextShapingMethod)
		.TextFlowDirection(InArgs._TextFlowDirection)
	);

	OnClicked_WithPos = InArgs._OnClicked;
}

FReply SFDButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Unhandled();
	const EButtonClickMethod::Type InputClickMethod = GetClickMethodFromInputType(MouseEvent);
	const bool bMustBePressed = InputClickMethod == EButtonClickMethod::DownAndUp || InputClickMethod == EButtonClickMethod::PreciseClick;
	const bool bMeetsPressedRequirements = (!bMustBePressed || (IsPressed() && bMustBePressed));

	if (bMeetsPressedRequirements && ((MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || MouseEvent.IsTouchEvent())))
	{
		Release();

		if (IsEnabled())
		{
			if (InputClickMethod == EButtonClickMethod::MouseDown)
			{
				// NOTE: If we're configured to click on mouse-down/precise-tap, then we never capture the mouse thus
				//       may never receive an OnMouseButtonUp() call.  We make sure that our bIsPressed
				//       state is reset by overriding OnMouseLeave().
			}
			else
			{
				bool bEventOverButton = IsHovered();

				FVector2D Position = MouseEvent.GetScreenSpacePosition();

				if (!bEventOverButton && MouseEvent.IsTouchEvent())
				{
					bEventOverButton = MyGeometry.IsUnderLocation(Position);
				}

				if (bEventOverButton)
				{
					// If we asked for a precise tap, all we need is for the user to have not moved their pointer very far.
					const bool bTriggerForTouchEvent = InputClickMethod == EButtonClickMethod::PreciseClick;

					// If we were asked to allow the button to be clicked on mouse up, regardless of whether the user
					// pressed the button down first, then we'll allow the click to proceed without an active capture
					const bool bTriggerForMouseEvent = (InputClickMethod == EButtonClickMethod::MouseUp || HasMouseCapture());

					if ((bTriggerForTouchEvent || bTriggerForMouseEvent) && OnClicked_WithPos.IsBound() == true)
					{
						Reply = OnClicked_WithPos.Execute(Position);
					}
				}
			}
		}

		//If the user of the button didn't handle this click, then the button's
		//default behavior handles it.
		if (Reply.IsEventHandled() == false)
		{
			Reply = FReply::Handled();
		}
	}

	//If the user hasn't requested a new mouse captor and the button still has mouse capture,
	//then the default behavior of the button is to release mouse capture.
	if (Reply.GetMouseCaptor().IsValid() == false && HasMouseCapture())
	{
		Reply.ReleaseMouseCapture();
	}

	Invalidate(EInvalidateWidget::Layout);

	return Reply;
}


void SFDTriggerPanel::Construct( const FArguments& InArgs )
{
	FrameDataAsset = InArgs._FrameDataAsset;
	FrameAttribute = InArgs._Frame;
	CurrentFrame = FrameAttribute.Get();

	Update(true);
}

void SFDTriggerPanel::Update(bool bForce)
{
	uint32 NewFrame = FrameAttribute.Get();

	if ((!bForce && NewFrame == CurrentFrame) || !FrameDataAsset) return;
	CurrentFrame = NewFrame;

	TSharedPtr<SScrollBox> Box = SNew(SScrollBox);

	int NumTriggers = FrameDataAsset->GetFrameStruct(CurrentFrame).Triggers.Num();
	if (NumTriggers > 0)
	{
		for (int i = 0; i < NumTriggers; ++i)
		{
			FDetailsViewArgs Args;
			Args.bHideSelectionTip = true;

			UFDTrigger* Trigger = FrameDataAsset->GetFrameStruct(CurrentFrame).Triggers[i];
			FLinearColor TriggerEditorColor = Trigger->EditorColor;

			FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			TSharedPtr<IDetailsView> DetailsView = PropertyModule.CreateDetailView(Args);
			DetailsView->SetObject(Trigger);

			Box->AddSlot()
			.Padding(5)
			.AttachWidget
			(
				SNew(SBorder)
				.Padding(5)
				.BorderBackgroundColor(FSlateColor(TriggerEditorColor))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						DetailsView.ToSharedRef()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(2)
						.AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(30)
							.WidthOverride(30)
							[
								SNew(SButton)
								.Text(LOCTEXT("-", "-"))
								.IsEnabled(true)
								.OnClicked_Lambda
								(
									[this, i]()->FReply
									{
										FrameDataAsset->GetFrameStruct(CurrentFrame).Triggers.RemoveAt(i);
										Update(true);
										return FReply::Handled();
									}
								)
							]
						]
						+ SVerticalBox::Slot()
						.Padding(2)
						.AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(30)
							.WidthOverride(30)
							[
								SNew(SFDButton)
								.Text(LOCTEXT("+...", "+..."))							
								.IsEnabled(i == (NumTriggers - 1))
								.OnClicked(this, &SFDTriggerPanel::OnAddTriggerClicked)
							]
						]
					]
				]
			);
		}
	}
	else
	{
		Box->AddSlot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			.AttachWidget
			(
				SNew(SBox)
				.HeightOverride(30)
				.WidthOverride(120)
				[
					SNew(SFDButton)
					.Text(LOCTEXT("Add new Trigger...", "Add new Trigger..."))
					.IsEnabled(true)
					.OnClicked(this, &SFDTriggerPanel::OnAddTriggerClicked)
				]
			);
	}

	this->ChildSlot
	[
		Box.ToSharedRef()
	];
}

FReply SFDTriggerPanel::OnAddTriggerClicked(FVector2D CursorPos)
{
	TSharedPtr<SWidget> WidgetToFocus;
	WidgetToFocus = SummonContextMenu(CursorPos);

	return (WidgetToFocus.IsValid())
		? FReply::Handled().ReleaseMouseCapture().SetUserFocus(WidgetToFocus.ToSharedRef(), EFocusCause::SetDirectly)
		: FReply::Handled();
	
}

TSharedPtr<SWidget> SFDTriggerPanel::SummonContextMenu(FVector2D Position)
{
	const bool bCloseWindowAfterMenuSelection = true;
	TSharedPtr<FUICommandList> CommandList;
	FMenuBuilder MenuBuilder(bCloseWindowAfterMenuSelection, CommandList);
	FUIAction NewAction;

	MenuBuilder.BeginSection("FrameData Trigger", LOCTEXT("TriggerHeading", "Trigger"));
	{
		MakeNewTriggerPicker<UFDTrigger>(MenuBuilder);
	}
	MenuBuilder.EndSection(); //AnimNotify
	
	// Display the newly built menu
	FSlateApplication::Get().PushMenu(SharedThis(this), FWidgetPath(), MenuBuilder.MakeWidget(), Position, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	return TSharedPtr<SWidget>();
}

template<typename TriggerTypeClass>
void SFDTriggerPanel::MakeNewTriggerPicker(FMenuBuilder& MenuBuilder)
{
	class FFDTriggerClassFilter : public IClassViewerFilter
	{
	public:
		FFDTriggerClassFilter()
		{}

		bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			const bool bChildOfObjectClass = InClass->IsChildOf(TriggerTypeClass::StaticClass());
			const bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);
			return bChildOfObjectClass && bMatchesFlags;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions & InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
		{
			const bool bChildOfObjectClass = InUnloadedClassData->IsChildOf(TriggerTypeClass::StaticClass());
			const bool bMatchesFlags = !InUnloadedClassData->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);

			return bChildOfObjectClass && bMatchesFlags;
		}
	};

	// MenuBuilder always has a search widget added to it by default, hence if larger then 1 then something else has been added to it
	if (MenuBuilder.GetMultiBox()->GetBlocks().Num() > 1)
	{
		MenuBuilder.AddMenuSeparator();
	}

	FClassViewerInitializationOptions InitOptions;
	InitOptions.Mode = EClassViewerMode::ClassPicker;
	InitOptions.bShowObjectRootClass = false;
	InitOptions.bShowUnloadedBlueprints = true;
	InitOptions.bShowNoneOption = false;
	InitOptions.bEnableClassDynamicLoading = true;
	InitOptions.bExpandRootNodes = true;
	InitOptions.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	InitOptions.ClassFilter = MakeShared<FFDTriggerClassFilter>();
	InitOptions.bShowBackgroundBorder = false;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	MenuBuilder.AddWidget(
		SNew(SBox)
		.MinDesiredWidth(300.0f)
		.MaxDesiredHeight(400.0f)
		[
			ClassViewerModule.CreateClassViewer(InitOptions,
			FOnClassPicked::CreateLambda([this](UClass * InClass)
				{
					FSlateApplication::Get().DismissAllMenus();
					AddNewTrigger(InClass);
					Update(true);
				}
			))
		],
		FText(), true, false);
}

void SFDTriggerPanel::AddNewTrigger(UClass * InClass)
{
	if (!FrameDataAsset) return;

	FrameDataAsset->AddTrigger(InClass);
}

#undef LOCTEXT_NAMESPACE
