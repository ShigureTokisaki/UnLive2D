#include "UnLive2DAnimationBlueprintEditor.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "SBlueprintEditorToolbar.h"
#include "UnLive2DManagerModule.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "EditorReimportHandler.h"
#include "IUnLive2DToolkit.h"
#include "BlueprintEditor.h"
#include "SKismetInspector.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "IUnLive2DAssetFamily.h"
#include "Framework/Commands/GenericCommands.h"
#include "UnLive2DAssetEditorModeManager.h"
#include "SUnLive2DAnimBlueprintEditorViewport.h"
#include "UnLive2DMotion.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UnLive2DEditorStyle.h"

const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode("GraphName");
const FName FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode("Interface");

#define LOCTEXT_NAMESPACE "UnLive2DAnimationBlueprintEditor"

namespace UnLive2DAnimationBlueprintEditorTabs
{
	const FName DetailsTab(TEXT("DetailsTab"));
	const FName ViewportTab(TEXT("Viewport"));
	const FName AssetBrowserTab(TEXT("SequenceBrowser"));
	const FName CurveNamesTab(TEXT("AnimCurveViewerTab"));
	const FName GraphDocumentTab(TEXT("GraphDocumentTab"));
};

FUnLive2DAnimationBlueprintEditor::FUnLive2DAnimationBlueprintEditor()
	: UnLive2DAnimBlueprintEdited(nullptr)
{
}

FUnLive2DAnimationBlueprintEditor::~FUnLive2DAnimationBlueprintEditor()
{
	GEditor->OnBlueprintPreCompile().RemoveAll(this);

	GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetPostImport.RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);
}

void FUnLive2DAnimationBlueprintEditor::UndoAction()
{
	GEditor->UndoTransaction();
}

void FUnLive2DAnimationBlueprintEditor::RedoAction()
{
	GEditor->RedoTransaction();
}

FName FUnLive2DAnimationBlueprintEditor::GetToolkitFName() const
{
	return FName("UnLive2DAnimationBlueprintEditor");
}

FText FUnLive2DAnimationBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "UnLive2DAnimation Blueprint Editor");
}

FText FUnLive2DAnimationBlueprintEditor::GetToolkitName() const
{
	return FText::FromString(UnLive2DAnimBlueprintEdited->GetName());
}

FText FUnLive2DAnimationBlueprintEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(UnLive2DAnimBlueprintEdited);
}

FString FUnLive2DAnimationBlueprintEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "UnLive2DAnimation Blueprint Editor").ToString();
}

FLinearColor FUnLive2DAnimationBlueprintEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor( 0.5f, 0.25f, 0.35f, 0.5f );
}

void FUnLive2DAnimationBlueprintEditor::PostUndo(bool bSuccess)
{
	// PostUndo broadcast
	OnPostUndo.Broadcast();

	//RefreshPreviewInstanceTrackCurves();

	//ClearupPreviewMeshAnimNotifyStates();

	OnPostCompile();
}

void FUnLive2DAnimationBlueprintEditor::PostRedo(bool bSuccess)
{

	OnPostUndo.Broadcast();

	//ClearupPreviewMeshAnimNotifyStates();

	OnPostCompile();
}

void FUnLive2DAnimationBlueprintEditor::OnPostCompile()
{
	
}


void FUnLive2DAnimationBlueprintEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UnLive2DAnimationBlueprintEditor", "UnLive2DAnimation Blueprint Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	DocumentManager->SetTabManager(InTabManager);

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::ViewportTab, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTabTitle", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab, FOnSpawnTab::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::SpawnTab_AssetBrowser))
		.SetDisplayName(NSLOCTEXT("PersonaModes", "AssetBrowserTabTitle", "Asset Browser"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.TabIcon"));

	InTabManager->RegisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::GraphDocumentTab, FOnSpawnTab::CreateSP(this,&FUnLive2DAnimationBlueprintEditor::SpawnTab_GraphDocument))
		.SetDisplayName(LOCTEXT("AnimationBlueprintTitle", "Animation Blueprint"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUnLive2DEditorStyle::GetStyleSetName(), "ClassIcon.UnLive2DAnimBlueprint"));
}

void FUnLive2DAnimationBlueprintEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::ViewportTab);
	InTabManager->UnregisterTabSpawner(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab);
}


void FUnLive2DAnimationBlueprintEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(UnLive2DAnimBlueprintEdited);
}

class UUnLive2DAnimBlueprint* FUnLive2DAnimationBlueprintEditor::GetBlueprintObj() const
{
	return UnLive2DAnimBlueprintEdited;
}

void FUnLive2DAnimationBlueprintEditor::BindCommands()
{
	
}

void FUnLive2DAnimationBlueprintEditor::HandleViewportCreated(const TSharedRef<class SCompoundWidget>& InPersonaViewport)
{

}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		[
			SNew(SUnLive2DAnimBlueprintEditorViewport)
			.UnLive2DAnimBlueprintEdited(this, &FUnLive2DAnimationBlueprintEditor::GetBlueprintObj)
		];
}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_AssetBrowser(const FSpawnTabArgs& Args)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2DMotion::StaticClass()->GetFName());
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::AssetBrowser_OnMotionDoubleClicked);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::AssetBrowser_FilterMotionBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;

	//AssetPickerConfig.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);

	return SNew(SDockTab)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
}

TSharedRef<SDockTab> FUnLive2DAnimationBlueprintEditor::SpawnTab_GraphDocument(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab);

	if (UnLive2DAnimBlueprintGraphEditor.IsValid())
	{
		SpawnedTab->SetContent(UnLive2DAnimBlueprintGraphEditor.ToSharedRef());
	}

	return SpawnedTab;
}

TSharedRef<SGraphEditor> FUnLive2DAnimationBlueprintEditor::CreateGraphEditorWidget()
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList);
	}

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText_AnimationBlueprint", "ANIMATION BLUEPRINT");

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::OnSelectedNodesChanged);
	InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::OnNodeTitleCommitted);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FUnLive2DAnimationBlueprintEditor::PlaySingleNode);

	return  SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(UnLive2DAnimBlueprintEdited->GetGraph())
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

void FUnLive2DAnimationBlueprintEditor::ExtendMenu()
{
	if (MenuExtender.IsValid())
	{
		RemoveMenuExtender(MenuExtender);
		MenuExtender.Reset();
	}

	MenuExtender = MakeShareable(new FExtender);
	AddMenuExtender(MenuExtender);

	/*// add extensible menu if exists
	FAnimationBlueprintEditorModule& AnimationBlueprintEditorModule = FModuleManager::LoadModuleChecked<FAnimationBlueprintEditorModule>("AnimationBlueprintEditor");
	AddMenuExtender(AnimationBlueprintEditorModule.GetMenuExtensibilityManager()->GetAllExtenders(GetToolkitCommands(), GetEditingObjects()));*/
}

void FUnLive2DAnimationBlueprintEditor::ExtendToolbar()
{
	// If the ToolbarExtender is valid, remove it before rebuilding it
	if (ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}

	ToolbarExtender = MakeShareable(new FExtender);

	AddToolbarExtender(ToolbarExtender);

	ToolbarExtender->AddToolBarExtension
	(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ParentToolbarBuilder)
	{
		FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
		TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(UnLive2DAnimBlueprintEdited);
		AddToolbarWidget(MangerModule.CreateAssetFamilyShortcutWidget(SharedThis(this), AssetFamily));
	}));
}

void FUnLive2DAnimationBlueprintEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{

}

void FUnLive2DAnimationBlueprintEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameNode", "Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void FUnLive2DAnimationBlueprintEditor::PlaySingleNode(UEdGraphNode* Node)
{

}

void FUnLive2DAnimationBlueprintEditor::AssetBrowser_OnMotionDoubleClicked(const FAssetData& AssetData) const
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(RawAsset))
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Motion);
		}
	}
}

bool FUnLive2DAnimationBlueprintEditor::AssetBrowser_FilterMotionBasedOnParentClass(const FAssetData& AssetData) const
{
	if (!AssetData.IsValid()) return true;

	UUnLive2DMotion* TargetUnLive2DMotion = Cast<UUnLive2DMotion>(AssetData.GetAsset());

	if (UUnLive2DAnimBlueprint* AnimBlueprintPtr = GetBlueprintObj())
	{
		if (TargetUnLive2DMotion)
		{
			return !(TargetUnLive2DMotion->UnLive2D == AnimBlueprintPtr->TargetUnLive2D);
		}
	}

	return true;
}

void FUnLive2DAnimationBlueprintEditor::InitUnLive2DAnimationBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UUnLive2DAnimBlueprint* InAnimBlueprint)
{
	UnLive2DAnimBlueprintEdited = InAnimBlueprint;

	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	UnLive2DManagerToolkit = MangerModule.CreatePersonaToolkit(InAnimBlueprint);

	TSharedRef<IUnLive2DAssetFamily> AssetFamily = MangerModule.CreatePersonaAssetFamily(InAnimBlueprint);
	AssetFamily->RecordAssetOpened(FAssetData(InAnimBlueprint));

	// Build up a list of objects being edited in this asset editor

	TSharedPtr<FUnLive2DAnimationBlueprintEditor> ThisPtr(SharedThis(this));
	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(ThisPtr);
	}

	TArray<UObject*> AnimBlueprints;
	AnimBlueprints.Add(InAnimBlueprint);


	UnLive2DAnimBlueprintGraphEditor = CreateGraphEditorWidget();

	BindCommands();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Stanalone_UnLive2DAnimBlueprintEditorMode_Layout_v1.02")
	->AddArea
	(
		{
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			/*->Split
			(
				// Top toolbar
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)*/
			->Split
			(
				{
					// Main application area
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Horizontal)
					->Split
					(
						// Left top - viewport
						{
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.25f)
							->SetOrientation(Orient_Vertical)
							->Split
							(
								{
									// Left top - viewport
									FTabManager::NewStack()
									->SetSizeCoefficient(0.5f)
									->SetHideTabWell(true)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::ViewportTab, ETabState::OpenedTab)
								}
							)
							/*->Split
							(
								{
									//	Left bottom - preview settings
									FTabManager::NewStack()
									->SetSizeCoefficient(0.5f)
								//->AddTab(UnLive2DAnimationBlueprintEditorTabs::CurveNamesTab, ETabState::OpenedTab)
								->AddTab(FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab)
							}
							)*/
						}
					)
					->Split
					(
						// Middle 
						{
							FTabManager::NewSplitter()
							->SetOrientation(Orient_Vertical)
							->SetSizeCoefficient(0.55f)
							->Split
							(
								// Middle top - document edit area
								FTabManager::NewStack()
								->SetSizeCoefficient(0.8f)
								->SetHideTabWell(true)
								->AddTab(UnLive2DAnimationBlueprintEditorTabs::GraphDocumentTab, ETabState::OpenedTab)
							)
							/*->Split
							(
								// Middle bottom - compiler results & find
								FTabManager::NewStack()
								->SetSizeCoefficient(0.2f)
								->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
								->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
							)*/
						}
					)
					->Split
					(
						// Right side
						{
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.2f)
							->SetOrientation(Orient_Vertical)
							/*->Split
							(
								// Right top - selection details panel & overrides
								FTabManager::NewStack()
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.5f)
								->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
							)*/
							->Split
							(
								// Right bottom - Asset browser & advanced preview settings
								FTabManager::NewStack()
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.5f)
								->AddTab(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab, ETabState::OpenedTab)
							)
						}
					)
				}
			)
		}
	);


	const FName UnLive2DAnimationBlueprintEditorAppName(TEXT("UnLive2DAnimationBlueprintEditorApp"));

	// Initialize the asset editor and spawn tabs
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	InitAssetEditor(Mode, InitToolkitHost, UnLive2DAnimationBlueprintEditorAppName, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InAnimBlueprint);

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

}

#undef LOCTEXT_NAMESPACE