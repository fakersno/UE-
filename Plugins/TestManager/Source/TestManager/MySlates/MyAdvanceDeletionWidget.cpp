// Vince Petrelli All Rights Reserverd

//Ϊ�˴����Զ����SlateС��������Ҫ�Ƚ�һ������
#include "MyAdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "MyDebugHeader.h"
#include "TestManager.h"

#define ListAll TEXT("List All Available Assets")
#define  ListUnused TEXT("List Unused Assets")
#define ListSameName TEXT("List Assets With Same Name")
void SMyAdvanceDeletionTab::Construct(const FArguments& InArgs)//���캯��
{
	///**С�����Ƿ�֧�ּ��̽���*/
	bCanSupportFocus = true;

	FSlateFontInfo TitleTextFont = GetEmboseedTextFont();//��������
	TitleTextFont.Size = 32;
	MyStoredAssetsData = InArgs._MyAssetsDataToStore;
	MyDisplayedAssetsData = MyStoredAssetsData;
	CheckBoxesArray.Empty();
	AssetDataDeleteArray.Empty();
	ComboBoxSoureItems.Empty();
	ComboBoxSoureItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSoureItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSoureItems.Add(MakeShared<FString>(ListSameName));
	ChildSlot
		[
			//InArgs._TestString ��ȡ�� TestString ������ֵ
			SNew(SVerticalBox)//�����ı���,//Main vertical box
			+ SVerticalBox::Slot()//�Ӳ��
			.AutoHeight()//���������С�Զ��߶�
			[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("My first Advantance Dletion")))
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::Blue)
			.Font(TitleTextFont)
			]
			+SVerticalBox::Slot()//SecondSlot��������ָ���б������Ͱ����ı�
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					ConstructComBox()//����ѡ���
				]
				+SHorizontalBox::Slot()
				.FillWidth(.6f)//���ÿռ䰴��������
				[
					ConstructComboHelpTexts(TEXT("Specify the listing condition in the drop down. Left mouse click to go to where asset is located"),ETextJustify::Center )
				]
				+ SHorizontalBox::Slot()
					.FillWidth(.1f)
				[
						ConstructComboHelpTexts(TEXT("Current Folder:\n") + InArgs._CurrentSelectedFloder, ETextJustify::Right)//��·��
				]
			]
			+SVerticalBox::Slot()//�ʲ��б�ĵ�������λ
			.VAlign(VAlign_Fill)//�øò�λ�����ݳ�����ֱ����Ŀ��ÿռ�
			[
				SNew(SScrollBox)//������
				+SScrollBox::Slot()
				[
					ConstructAssetListView()//�б�����
				]
			]
			+ SVerticalBox::Slot()//���ĸ���ۣ�������3����ť
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(10.0f)
					.Padding(5.0f)
					[
					ConstructDeleteAllButton()
					]
					+ SHorizontalBox::Slot()
					.FillWidth(10.0f)
					.Padding(5.0f)
					[
					ConstructSelectButton()
					]
					+ SHorizontalBox::Slot()
					.FillWidth(10.0f)//��۵Ŀ�ȱ���Ϊ 10.0
					.Padding(5.0f)//���ֵ
					[
						ConstructDeselectAllButton()
					]
			]
		];
}
TSharedRef<SListView<TSharedPtr<FAssetData>>> SMyAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)//Slate����
		.ItemHeight(24.f)
		.ListItemsSource(&MyDisplayedAssetsData)
		.OnGenerateRow(this, &SMyAdvanceDeletionTab::OnGenerateRowForList)//һ�е���һ�������������б��еĻص�����ΪSMyAdvanceDeletionTab���е�OnGenerateRowForList������
		.OnMouseButtonClick(this, &SMyAdvanceDeletionTab::OnRowWidgetMouseButtonClicker);
	return ConstructedAssetListView.ToSharedRef();
}

#pragma region ComboBoxForListingCondition
TSharedRef<SComboBox<TSharedPtr<FString>>> SMyAdvanceDeletionTab::ConstructComBox()
{

	TSharedRef<SComboBox<TSharedPtr<FString>>>ConstrucedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSoureItems)
		.OnGenerateWidget(this, &SMyAdvanceDeletionTab::OnGenerateComboContent)//����ʱ����
		.OnSelectionChanged(this, &SMyAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];
	return ConstrucedComboBox;
}
TSharedRef<SWidget> SMyAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)//��ʾ��������
{
	TSharedRef<STextBlock>ConstructedComboText = SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));
	return ConstructedComboText;
}
void SMyAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo0)//��������¼�
{

	//MyDebugHeader::print(*SelectedOption.Get(), FColor::Cyan);

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));
	FTestManagerModule& TestManagerModule = FModuleManager::LoadModuleChecked<FTestManagerModule>(TEXT("TestManager"));
	if (*SelectedOption.Get() == ListAll) {
		MyDisplayedAssetsData = MyStoredAssetsData;
		RefreshAssetListView();

	} 
	else if (*SelectedOption.Get()==ListUnused) {
		TestManagerModule.ListUnusedAssetsForAssetList(MyStoredAssetsData, MyDisplayedAssetsData);
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListSameName) {
		TestManagerModule.ListSameNameAssetsForAssetList(MyStoredAssetsData, MyDisplayedAssetsData);
		RefreshAssetListView();
	}
}
TSharedRef<STextBlock> SMyAdvanceDeletionTab::ConstructComboHelpTexts(const FString& TestContent, ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock>ConstructedHelpText = SNew(STextBlock)
		.Text(FText::FromString(TestContent))
		.Justification(TextJustify)
		.AutoWrapText(true);//�Զ�����
		return ConstructedHelpText;
}
#pragma  endregion
#pragma region RowWidgetForAssetListView
TSharedRef<ITableRow> SMyAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase> &OwnerTable)//ÿ�д���һ��
{
	if (!AssetDataToDisplay->IsValid())return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);//���б�ؼ�
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClass.ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 10;
	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;
	TSharedRef<STableRow<TSharedPtr<FAssetData>>>ListViewRowWidet =//���ÿ��UI
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(5.f))//.Padding(FMargin(5.f))�����ñ���е��ڱ߾࣬���ڿؼ���Ե������֮������ϡ��¡����Ҹ� 5 ����λ���ȵĿհ����������Ӿ��ϵļ��Ч����
		[
			SNew(SHorizontalBox)
			//ˮƽ���һ�����
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)//���ɹ�ѡ��
			]
			//Second slot for displaying asset class name
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			.FillWidth(.5f)
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)//��������
			]
			//Third slot for displaying asset name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				ConstructTextForRowWidget(DisplayAssetName,AssetNameFont)//�����������
			]
			//Fourth slot for a button
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
				[
					ConstructButtonForRowWidget(AssetDataToDisplay)//���ɰ�ť
 			]

		];
	return ListViewRowWidet;
}

void SMyAdvanceDeletionTab::RefreshAssetListView()//���¹����б�
{
	AssetDataDeleteArray.Empty();
	if (ConstructedAssetListView.IsValid()) {
		ConstructedAssetListView->RebuildList();
	}
}

void SMyAdvanceDeletionTab::OnRowWidgetMouseButtonClicker(TSharedPtr<FAssetData> ClickedData)//ת����Դ��������Ӧ��Դ
{
	FTestManagerModule& TestManagerModule = FModuleManager::LoadModuleChecked<FTestManagerModule>("TestManager");
	TestManagerModule.MyCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());
}

TSharedRef<SCheckBox> SMyAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox>ConstrucedCheckBox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SMyAdvanceDeletionTab::OnCheckBoxStateChangeg, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstrucedCheckBox);
	return ConstrucedCheckBox;
}

void SMyAdvanceDeletionTab::OnCheckBoxStateChangeg(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	//�����Ƿ�ѡ��Ϊ���������Դ
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		//MyDebugHeader::print(AssetData->AssetName.ToString() + TEXT(" is unchecked"),FColor::Red);
		if (AssetDataDeleteArray.Contains(AssetData)) {
			AssetDataDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		//MyDebugHeader::print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		AssetDataDeleteArray.AddUnique(AssetData);

		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}
#pragma endregion
#pragma region TabButtons
TSharedRef<STextBlock> SMyAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo FontToUse)
{
	TSharedRef<STextBlock>ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);
	return ConstructedTextBlock;
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetData)
{
	TSharedRef<SButton>ConstructedButton = SNew(SButton)
		.Text(FText::FromString(TEXT("MyDelete")))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnDeleteButtonClicked, AssetData);

	return ConstructedButton;
}

FReply SMyAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	//MyDebugHeader::print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"), FColor::Red);
	FTestManagerModule& MyTestMangerModule = FModuleManager::LoadModuleChecked <FTestManagerModule>(TEXT("TestManager"));
	const bool bAssetDeleted = MyTestMangerModule.MyDeleteSigleAssetForAssetList(*ClickedAssetData.Get());
	if (bAssetDeleted) {
		if (MyStoredAssetsData.Contains(ClickedAssetData)) {
			MyStoredAssetsData.Remove(ClickedAssetData);
		}
		if (MyDisplayedAssetsData.Contains(ClickedAssetData))
		{
			MyDisplayedAssetsData.Remove(ClickedAssetData);
		}
		RefreshAssetListView();
	}

	return FReply::Handled();
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructDeleteAllButton()//ɾ����Դ��
{
	TSharedRef<SButton>DeleteAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnDeleteAllButtonClicked);
	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));//�����ı�
	return DeleteAllButton;
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructSelectButton()
{
	TSharedRef<SButton>SelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnSelectedAllButtonClicked);
	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));
	return SelectAllButton;	
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton>DeselectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnDeselectAllButtonClicked);
	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));
	return DeselectAllButton;
}

FReply SMyAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	//MyDebugHeader::print(TEXT("Delete All Button Clicked"), FColor::Cyan);
	if (AssetDataDeleteArray.Num() == 0)
	{
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}
	TArray<FAssetData>AssetDataToDelete;
	for (const TSharedPtr<FAssetData>& Data : AssetDataDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}
	FTestManagerModule& TestMangerModule = FModuleManager::LoadModuleChecked<FTestManagerModule>("TestManager");
	const bool bAssetsDeleted = TestMangerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);//����ģ��ִ��ɾ������
	if (bAssetsDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeletedData : AssetDataDeleteArray)//ȥ���Ѿ�ɾ���ĺ��������ã��������ü���
		{
			if (MyStoredAssetsData.Contains(DeletedData))
			{
				MyStoredAssetsData.Remove(DeletedData);
			}
			if (MyDisplayedAssetsData.Contains(DeletedData))
			{
				MyDisplayedAssetsData.Remove(DeletedData);
			}
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SMyAdvanceDeletionTab::OnSelectedAllButtonClicked()
{
	//MyDebugHeader::print(TEXT("Select All Button Clicked"), FColor::Cyan);
	if (CheckBoxesArray.Num() == 0)return FReply::Handled();
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray) {
		if (!CheckBox->IsChecked()) {//Ϊfalse
			CheckBox->ToggleCheckedState();//�л�true״̬
		}
	}

	return FReply::Handled();
}

FReply SMyAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	//MyDebugHeader::print(TEXT("Deselect All Button Clicked"), FColor::Cyan);
	if (CheckBoxesArray.Num() == 0)return FReply::Handled();
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray) {
		if (CheckBox->IsChecked()) {//Ϊtrue
			CheckBox->ToggleCheckedState();//��Ϊfalse
		}
	}

	return FReply::Handled();
}


TSharedRef<STextBlock> SMyAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextInfo = GetEmboseedTextFont();
	ButtonTextInfo.Size=15.f;
	TSharedRef<STextBlock>ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextInfo)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion
