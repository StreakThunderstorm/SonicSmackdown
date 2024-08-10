#include "MNNode_SEdNode.h"
#include "../MoveNetworkEditorPCH.h"
#include "MNColors.h"
#include "SLevelOfDetailBranchNode.h"
#include "SInlineEditableTextBlock.h"
#include "SCommentBubble.h"
#include "SlateOptMacros.h"
#include "SGraphPin.h"
#include "GraphEditorSettings.h"
#include "MNNode_EdNode.h"

#define LOCTEXT_NAMESPACE "EdNode_MoveNetwork"

//////////////////////////////////////////////////////////////////////////
class SMNPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SMNPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel = true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &SMNPin::GetPinBorder)
			.BorderBackgroundColor(this, &SMNPin::GetPinColor)
			.OnMouseButtonDown(this, &SMNPin::OnPinMouseDown)
			.Cursor(this, &SMNPin::GetPinCursor)
			.Padding(FMargin(10.0f))
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		return MNColors::Pin::Default;
	}

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	const FSlateBrush* GetPinBorder() const
	{
		return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Body"));
	}
};


//////////////////////////////////////////////////////////////////////////
void MNNode_SEDNode::Construct(const FArguments& InArgs, UMNNode_EdNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
	InNode->SEdNode = this;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void MNNode_SEDNode::UpdateGraphNode()
{
	const FMargin NodePadding = FMargin(2.0f);

	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	OutputPinBox.Reset();

	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0.0f)
			.BorderBackgroundColor(this, &MNNode_SEDNode::GetBorderBackgroundColor)
			[
				SNew(SOverlay)

				// Pins and node details
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SVerticalBox)

					// INPUT PIN AREA
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(NodePadding.Top)
						[
							SAssignNew(LeftNodeBox, SVerticalBox)
						]
					]

					// STATE NAME AREA
					+ SVerticalBox::Slot()
					.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(NodeBody, SBorder)
							.BorderImage(FEditorStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
							.BorderBackgroundColor(this, &MNNode_SEDNode::GetBackgroundColor)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							.Visibility(EVisibility::SelfHitTestInvisible)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											// POPUP ERROR MESSAGE
											SAssignNew(ErrorText, SErrorText)
											.BackgroundColor(this, &MNNode_SEDNode::GetErrorColor)
											.ToolTipText(this, &MNNode_SEDNode::GetErrorMsgToolTip)
										]

										+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SAssignNew(InlineEditableText, SInlineEditableTextBlock)
													.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
													.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
													.OnVerifyTextChanged(this, &MNNode_SEDNode::OnVerifyNameTextChanged)
													.OnTextCommitted(this, &MNNode_SEDNode::OnNameTextCommited)
													.IsReadOnly(this, &MNNode_SEDNode::IsNameReadOnly)
													.IsSelected(this, &MNNode_SEDNode::IsSelectedExclusively)
												]
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													NodeTitle.ToSharedRef()
												]
											]
										]
									]
								]
							]
						]
					]

					// OUTPUT PIN AREA
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(NodePadding.Bottom)
						[
							SAssignNew(RightNodeBox, SVerticalBox)
							+ SVerticalBox::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(20.0f, 0.0f)
							.FillHeight(1.0f)
							[
								SAssignNew(OutputPinBox, SHorizontalBox)
							]
						]
					]
				]
			]
		];

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

void MNNode_SEDNode::CreatePinWidgets()
{
	UMNNode_EdNode* StateNode = CastChecked<UMNNode_EdNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SMNPin, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void MNNode_SEDNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(20.0f,0.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		OutputPinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(1.0f)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
}

bool MNNode_SEDNode::IsNameReadOnly() const
{
	UMNNode_EdNode* EdNode_Node = Cast<UMNNode_EdNode>(GraphNode);
	check(EdNode_Node != nullptr);

	UMoveNetwork* MoveNetwork = EdNode_Node->MoveNetworkNode->Graph;
	check(MoveNetwork != nullptr);

	return !MoveNetwork->bCanRenameNode || SGraphNode::IsNameReadOnly();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void MNNode_SEDNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UMNNode_EdNode* MyNode = CastChecked<UMNNode_EdNode>(GraphNode);

	if (MyNode != nullptr && MyNode->MoveNetworkNode != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("MoveNetworkEditorRenameNode", "Move Network Editor: Rename Node"));
		MyNode->Modify();
		MyNode->MoveNetworkNode->Modify();
		MyNode->MoveNetworkNode->SetNodeTitle(InText);
		UpdateGraphNode();
	}
}

FSlateColor MNNode_SEDNode::GetBorderBackgroundColor() const
{
	UMNNode_EdNode* MyNode = CastChecked<UMNNode_EdNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : MNColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor MNNode_SEDNode::GetBackgroundColor() const
{
	return MNColors::NodeBody::Default;
}

EVisibility MNNode_SEDNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* MNNode_SEDNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

#undef LOCTEXT_NAMESPACE
