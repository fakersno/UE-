// Vince Petrelli All Rights Reserverd


#include "MyQuickMaterialCreationWidget.h"
#include "MyDebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#pragma region MyQuickMaterialCreationCore

void UMyQuickMaterialCreationWidget::CreateMaterialFromSelectedTexture()
{
	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));
			return;
		} 
	}
	TArray<FAssetData>SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*>SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;
	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath))return;//�ļ���·��
	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName))return;//��֤���ʲ������Ƿ��������ʲ�ͬ��

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);//��������
	if (!CreatedMaterial) {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
	}


	for (UTexture2D* SelectedTexture : SelectedTexturesArray)//ÿ�����������һ��
	{
		if (!SelectedTexture) continue;

		Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
	}
	//MyDebugHeader::print(SelectedTextureFolderPath, FColor::Cyan);
	if (PinsConnectedCounter > 0)
	{
		MyDebugHeader::showNotifyInfo(TEXT("Successfully conntected ") + FString::FromInt(PinsConnectedCounter) + TEXT("pins"));
	}
}

//����ѡ�е����ݣ������˵��������δѡ���򷵻�false
bool UMyQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProccess, TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProccess.Num() == 0)
	{
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No textures selected"));
		return false;
	}
	bool bMaterialNameSet = false;
	for (const FAssetData& SelectData : SelectedDataToProccess) 
	{
		UObject* SelectAsset = SelectData.GetAsset();
		if (!SelectAsset)continue;
		UTexture2D* SelectTexture = Cast<UTexture2D>(SelectAsset);
		if (!SelectTexture)
		{
			MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only textures\n")+SelectAsset->GetName()+TEXT("is not a texture"));
			return false;
		}
		OutSelectedTexturesArray.Add(SelectTexture);
		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectData.PackagePath.ToString();//��ȡ���ڰ����ļ���·��
		}
		if (!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet = true;
		}

	}
	return true;
}
//����������Ʊ�ָ���ļ����µ���Դʹ�ã��򷵻�true
bool UMyQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{

	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);

	for (const FString& ExistingAssetPath : ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath); //���ڻ�ȡָ��·���Ļ����ļ�������ȥ��·������չ������ļ�������

		if (ExistingAssetName.Equals(MaterialNameToCheck))
		{
			MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck +
				TEXT(" is already used by asset"));

			return true;
		}
	}

	return false;
}
UMaterial* UMyQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();//ʵ�ʴ������ʵĹ�����
	UObject* CreateObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial, PathToPutMaterial, UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreateObject);


}
void UMyQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);//����ɼ���
	if (!TextureSampleNode)return;
	if (!CreatedMaterial->BaseColor.IsConnected())//BaseColor�ڵ�δ����
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))//���ӽڵ�
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->Metallic.IsConnected())
	{
		if (TryConnectMetalic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
}
#pragma endregion


#pragma region MyCreateMaterialNodesConnectPings
bool UMyQuickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))//��ѡ�еĲ����а���BaseColor����
		{
			TextureSampleNode->Texture = SelectedTexture;//
			CreatedMaterial->Expressions.Add(TextureSampleNode);//������ɼ�����ӵ�������ȥ
			CreatedMaterial->BaseColor.Expression = TextureSampleNode;//��������ɼ���
			CreatedMaterial->PostEditChange();//֪ͨ�༭�������Ѿ������˸ı䡣
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			return true;
		}

	}
	return false;
}
bool UMyQuickMaterialCreationWidget::TryConnectMetalic(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& MetaliName : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetaliName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;//��������ѹ����ʽ
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;//����ɼ�����
			CreatedMaterial->Expressions.Add(TextureSampleNode);
			CreatedMaterial->Metallic.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240;
		}
	}
	return false;
}

#pragma  endregion