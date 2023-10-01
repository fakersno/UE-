// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestManager.h"
#include "ContentBrowserModule.h"
#include "MyDebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistryModule.h"//�ʲ��ض���ģ��
#include "AssetToolsModule.h"//�ʲ�����ģ��
#include "TestManager/MySlates/MyAdvanceDeletionWidget.h"
#include <UnrealEdSharedPCH.h>
#include <Widgets/Docking/SDockTab.h>
#include "MyTestMangerStyle.h"

#define LOCTEXT_NAMESPACE "FTestManagerModule"

void FTestManagerModule::StartupModule()
{
	FTestMangerStyle::InitializeIcons();

	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

#pragma region ContentBrowserMenuExtention


void FTestManagerModule::InitCBMenuExtention()//��ʼ���Զ���˵�
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//���ڻ�ȡ���е�·����ͼ��Path View���Ĳ˵���չ�����ݽṹ��
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	/*��һ�ְ�
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FTestManagerModule::CustomCBMenuExtender);
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);*/
	//�ڶ���
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FTestManagerModule::CustomCBMenuExtender));//�Ҽ������Դ���ļ���ʱ������
}
TSharedRef<FExtender> FTestManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender>MenuExtender(new FExtender());//���ɲ˵���չ������
	if (SelectedPaths.Num() > 0) 
	{
		MenuExtender->AddMenuExtension(FName("Delete"),//��չ�������
			EExtensionHook::After,//��ʾ���������չ��֮������µĸ���չ��
			TSharedPtr<FUICommandList>(),//�������ڴ����Զ����ݼ�
			FMenuExtensionDelegate::CreateRaw(this, &FTestManagerModule::AddCBMenuEntry));//���������Զ������ˣ�������Ҽ������Դ���ļ���ʱ������
		MyFolderPathsSelected = SelectedPaths;//��ȡ��ǰ����ѡ����ļ�·��

	}
	return MenuExtender;
}
//������ɶ���Ĳ˵����ϸ��
void FTestManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("My  Delete Unused Assets")),//����
		FText::FromString(TEXT("My  Safely delete all unused assets under folder")),//����˵��
		FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.MyDeleteUnusedAssets"),//����ͼ��
		FExecuteAction::CreateRaw(this, &FTestManagerModule::OnDeleteUnsuedAssetButtonClicked));//���ʱ����

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all empty folders")),
		FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFloders"),
		FExecuteAction::CreateRaw(this, &FTestManagerModule::OnDeleteEmptyFoldersButtonClicked));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("MyAdvanence Deletion")),
		FText::FromString(TEXT("Tips:List assets by specific condition int a tab for dleting ")),
		FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.MyAdvanenceDeletion"),
		FExecuteAction::CreateRaw(this, &FTestManagerModule::OnAdvanenceDeletionButtonClicked));
}
void FTestManagerModule::OnDeleteUnsuedAssetButtonClicked()
{
	MyDebugHeader::print(TEXT("WorkingMyManager"), FColor::Green);
	if (MyFolderPathsSelected.Num() > 1) {//���ܴ���һ���ļ�
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("You can only do this to one folder"));
		return;
	}
	TArray<FString>AssetsPathNames = UEditorAssetLibrary::ListAssets(MyFolderPathsSelected[0]);//��ȡ�ļ���������Դ��·��
	if (AssetsPathNames.Num() == 0) {//û����Դ
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"),false);
		return;
	}
	EAppReturnType::Type ConfigResult = MyDebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of" )+ FString::FromInt(AssetsPathNames.Num())
		+ TEXT(" assets need to be checked.\nWould you like to procceed?"), false);
	if (ConfigResult == EAppReturnType::No)return;
	FixUpRedirectors();
	TArray<FAssetData>UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames) {//�������ļ����е���Դ����
		if (AssetPathName.Contains(TEXT("Developers"))||
			AssetPathName.Contains(TEXT("Collections"))||
			AssetPathName.Contains(TEXT("__ExternalActors__"))||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))continue;
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))continue;//���ָ������Դ·���Ƿ������Ч����Դ
		TArray<FString>AssetReferencers=UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);//�����·��������
		if (AssetReferencers.Num() == 0) {//����Ϊ0
			const FAssetData UnusedAssetData=UEditorAssetLibrary::FindAssetData(AssetPathName);
				UnusedAssetsDataArray.Add(UnusedAssetData);
		}

	}
	if (UnusedAssetsDataArray.Num() > 0) {
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false);
	}
}
void FTestManagerModule::OnDeleteEmptyFoldersButtonClicked()//ɾ���ļ���
{
	//MyDebugHeader::print(TEXT("haha"), FColor::Green);
	FixUpRedirectors();
	//��һ�� true ������ʾ�������ļ��У��ڶ��� true ������ʾ�������ļ����е���Դ��
	TArray<FString>FolderPathArray = UEditorAssetLibrary::ListAssets(MyFolderPathsSelected[0], true, true);
	uint32 Counter = 0;
	FString EmptyFolderNames;
	TArray<FString>EmptyFoldersPathArray;
	for (const FString& FolderPath : FolderPathArray) {
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath))continue;//���ָ�����ļ���·���Ƿ����
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath)) {
			EmptyFolderNames.Append(FolderPath);
			EmptyFolderNames.Append("\n");
			EmptyFoldersPathArray.Add(EmptyFolderNames);
		}
	}
	if (EmptyFoldersPathArray.Num() == 0) {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty found under selected folder"), false);
		return;
	}
	EAppReturnType::Type ConfirmResult = MyDebugHeader::ShowMsgDialog(EAppMsgType::OkCancel, TEXT("Empty folders found int :\n") + EmptyFolderNames + TEXT("\nWould you like to delete all?"),false);
	if (ConfirmResult == EAppReturnType::Cancel)return;
	for (const FString& EmpthyFolderPath : EmptyFoldersPathArray) {
		UEditorAssetLibrary::DeleteDirectory(EmpthyFolderPath)?
			Counter++ : MyDebugHeader::print(TEXT("Failed to delete") + EmpthyFolderPath, FColor::Red);
	}
	if (Counter > 0) {
		MyDebugHeader::showNotifyInfo(TEXT("Successfully deleted") + FString::FromInt(Counter) + TEXT("folders"));
	}

}
void FTestManagerModule::OnAdvanenceDeletionButtonClicked()//���ʱ����
{
	FixUpRedirectors();
	//MyDebugHeader::print(TEXT("Working "), FColor::Green);
	FGlobalTabmanager::Get()->TryInvokeTab(FName("MyAdvanceDeletion"));//������������ѡ�
}
void FTestManagerModule::FixUpRedirectors()//�ļ�������ָ�������ض���
{
	TArray<UObjectRedirector*>RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");
	TArray<FAssetData>OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);
	for (const FAssetData& RedirectroData : OutRedirectors) {
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectroData.GetAsset())) {
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	//MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Fix OK"), false);
}
#pragma endregion

#pragma region CustomEditroTab
void FTestManagerModule::RegisterAdvanceDeletionTab()//��Դ���������ѡ�ע���ʼ��
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("MyAdvanceDeletion"),//ע�ᶨ��ƥ�������
		FOnSpawnTab::CreateRaw(this, &FTestManagerModule::OnSpawnAdvanenceDeltionTab))//*��һ��ѡ�ʵ��������ʱ���á�
		.SetDisplayName(FText::FromString(TEXT("MyAdvance Deletion")))//����ѡ���ʾ������
		.SetIcon(FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.MyAdvanenceDeletion"));//ͼ��
}
//����ѡ�
TSharedRef<SDockTab> FTestManagerModule::OnSpawnAdvanenceDeltionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SMyAdvanceDeletionTab)
			.MyAssetsDataToStore(GetAllAssetDataUnderSelectedFolder())//��������Slate�еı�������
			.CurrentSelectedFloder(MyFolderPathsSelected.Num() <= 0?FString(): MyFolderPathsSelected[0])
		];
}
TArray<TSharedPtr<FAssetData>> FTestManagerModule::GetAllAssetDataUnderSelectedFolder()//��ȡ��Դ
{
	TArray<TSharedPtr<FAssetData>>MyAvaiableAssetsData;
	if (MyFolderPathsSelected.Num() <= 0)return MyAvaiableAssetsData;//��֮ǰ�ڱ༭���д�ѡ����رգ��´������༭�����Զ��򿪶�δѡ���ļ��е�������Խ�������
	TArray<FString>MyAssetsPathNames = UEditorAssetLibrary::ListAssets(MyFolderPathsSelected[0]);//��ȡָ���ļ�Ŀ¼�µ�������Դ·��
	for (const FString& AssetPathName : MyAssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))continue;
		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		MyAvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
		
	}
	return MyAvaiableAssetsData;
}
#pragma endregion

#pragma region ProcessDataForAdvanceDletionTab
bool FTestManagerModule::MyDeleteSigleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);
	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0) {
		return true;
	}
	return false;
}
bool FTestManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}
	return false;
}
void FTestManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter) 
	{
		TArray<FString>AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());
		if (AssetReferencers.Num() == 0) //û�����õ���Դ
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}
void FTestManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();
	TMultiMap<FString, TSharedPtr<FAssetData>>AssetsInfoMultiMap;
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter) {
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter) {
		TArray<TSharedPtr<FAssetData>>OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);//ͨ����Դ���Ʋ�����ͬ����Դ
		if (OutAssetsData.Num() <= 1)continue;
		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData) {
			if (SameNameData.IsValid()) {
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}
void FTestManagerModule::MyCBToClickedAssetForAssetList(const FString& AssetPathToSync)//��ת��Դ��������ָ����Դ
{
	TArray<FString>AssetsPathSync;
	AssetsPathSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathSync);
}
#pragma endregion
void FTestManagerModule::ShutdownModule()//�ر�ģ��
{
	//������������ڹػ�ʱ���������������ģ�顣����֧�ֶ�̬���ص�ģ�飬
//������ж��ģ��֮ǰ�������������
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//�ֶ��ر�ģ��
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("MyAdvanceDeletion"));
	FTestMangerStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTestManagerModule, TestManager)