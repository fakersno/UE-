// Vince Petrelli All Rights Reserverd
#include "MyQuickAssetAction.h"
#include "MyDebugHeader.h"//�Զ���֪ͨͷ�ļ�
#include "EditorUtilityLibrary.h"//�༭�����ܹ��߿�
#include "EditorAssetLibrary.h"//��Դ������
#include "Misc/MessageDialog.h"//��Ϣ֪ͨ��
#include "ObjectTools.h"//���󹤾�
#include "AssetRegistryModule.h"//�ʲ��ض���ģ��
#include "AssetToolsModule.h"//�ʲ�����ģ��

void UMyQuickAssetAction::MyTestFunc()
{
	MyDebugHeader::print(TEXT("working"), FColor::Cyan);
	MyDebugHeader::pringLog(TEXT("Working"));
}

void UMyQuickAssetAction::MyduplicateAssets(int32 NumOfDuplicates)//�ʲ���������
{
	if (NumOfDuplicates <= 0) {
		//MyDebugHeader::print(TEXT("Plaese enter a VALID number"), FColor::Red);
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Plase enter a VALID number"));
		return;
	}
	TArray<FAssetData>SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();//��ȡ�༭���е�ǰѡ�е��ʲ���Asset������Ϣ
	uint32 counter = 0;
	for (const FAssetData& SelectedAssetData : SelectedAssetsData) {
		for (int32 i = 0; i < NumOfDuplicates; i++) {
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName)) {
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				counter++;
			}
		}

	}
	if (counter > 0) {
		//MyDebugHeader::print(TEXT("Successfully duplicated" + FString::FromInt(counter) + "files"), FColor::Green);
		MyDebugHeader::showNotifyInfo(TEXT("Successfully duplicated" + FString::FromInt(counter) + "files")); 
	}
}


void UMyQuickAssetAction::MyAddPrefixes()//���ǰ׺
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 counter = 0;
	for (UObject* SelectedObject : SelectedObjects) {
		if (!SelectedObject)continue;
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());
		if (!PrefixFound || PrefixFound->IsEmpty()) {
			MyDebugHeader::print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}
		FString OldName = SelectedObject->GetName();
		if (OldName.StartsWith(* PrefixFound)) {//����Ƿ�������Ӧǰ׺
			MyDebugHeader::print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));//�Ƴ�һЩ����ʵ��֮ǰ����ǰ׺
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		++counter;
	}

		if (counter > 0)
		{
			MyDebugHeader::showNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(counter) + " assets"));
		}

}


void UMyQuickAssetAction::MyRemoveUnusedAssets()//�Ƴ���Դ
{
	TArray<FAssetData>SelcetAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData>UnusedAssetsData;
	MyFixUpRedirectors();
	for (const FAssetData& SelectAssetData : SelcetAssetsData) {
		TArray<FString>AssetRefrence = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectAssetData.ObjectPath.ToString());
		if (AssetRefrence.Num() == 0) {//û�����õ���Դ
			UnusedAssetsData.Add(SelectAssetData);
		}

	}
	if (UnusedAssetsData.Num() == 0) {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
	}
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);//ɾ���ʲ�
	if (NumOfAssetsDeleted == 0)return;
	MyDebugHeader::showNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}


void UMyQuickAssetAction::MyFixUpRedirectors()
{
	TArray<UObjectRedirector*>RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryMoudle = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));//���ڼ��� AssetRegistry ģ�鲢��ȡ��ʵ���Ĵ���
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");//��Game�ļ���������
	Filter.ClassNames.Emplace("ObjectRedirector");//ֻ���ض���UObject����Դ����Ҫ�ض���������ԴҪ����Texture2D��Ԫ��
	TArray<FAssetData>OutRedirectors;
	AssetRegistryMoudle.Get().GetAssets(Filter, OutRedirectors);//������������Ѱ�Ҷ���洢��������
	for (const FAssetData& RedirectorData : OutRedirectors) {
		//UObjectRedirector��UObject������
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset())) {
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	FAssetToolsModule& AssetToolsMoude = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsMoude.Get().FixupReferencers(RedirectorsToFixArray);//�ض����޸�
} 