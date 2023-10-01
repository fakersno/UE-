// Vince Petrelli All Rights Reserverd

#pragma once
#include"Widgets/SCompoundWidget.h"

class SMyAdvanceDeletionTab :public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SMyAdvanceDeletionTab) {}
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, MyAssetsDataToStore)//�������� Slate �ؼ����Զ������,�����ⲿ��ȡ
	SLATE_ARGUMENT(FString, CurrentSelectedFloder)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
private:
	TArray<TSharedPtr<FAssetData>>MyStoredAssetsData;
	TArray<TSharedPtr<FAssetData>>MyDisplayedAssetsData;
	TArray<TSharedRef<SCheckBox>>CheckBoxesArray;//��ѡ��
	TArray<TSharedPtr<FAssetData>>AssetDataDeleteArray;

	TSharedRef<SListView<TSharedPtr<FAssetData>>>ConstructAssetListView();//�����б���ͼ
	TSharedPtr<SListView<TSharedPtr<FAssetData>>>ConstructedAssetListView;
	void RefreshAssetListView();


#pragma region ComboBoxForListingCondition

	TSharedRef<SComboBox<TSharedPtr<FString>>>ConstructComBox();
	TArray<TSharedPtr<FString>>ComboBoxSoureItems;
	TSharedRef<SWidget>OnGenerateComboContent(TSharedPtr<FString>SourceItem);
	void OnComboSelectionChanged(TSharedPtr<FString>SelectedOption, ESelectInfo::Type InSelectInfo0);
	TSharedPtr<STextBlock>ComboDisplayTextBlock;
	TSharedRef<STextBlock>ConstructComboHelpTexts(const FString& TestContent, ETextJustify::Type TextJustify);
#pragma  endregion
#pragma region RowWidgetForAssetListView
	TSharedRef<ITableRow>OnGenerateRowForList(TSharedPtr<FAssetData>AssetDataToDisplay, const TSharedRef<STableViewBase>&OwnerTable);
	void OnRowWidgetMouseButtonClicker(TSharedPtr<FAssetData>ClickedData);
	TSharedRef<SCheckBox>ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChangeg(ECheckBoxState NewState, TSharedPtr<FAssetData>AssetData);
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo FontToUse);
	TSharedRef<SButton>ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetData);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData>ClickedAssetData);

#pragma endregion

#pragma region TabButtons
	TSharedRef<SButton>ConstructDeleteAllButton();
	TSharedRef<SButton>ConstructSelectButton();
	TSharedRef<SButton>ConstructDeselectAllButton();
	FReply OnDeleteAllButtonClicked();
	FReply OnSelectedAllButtonClicked();
	FReply OnDeselectAllButtonClicked();
	TSharedRef<STextBlock>ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion
	FSlateFontInfo GetEmboseedTextFont()const  { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }


};