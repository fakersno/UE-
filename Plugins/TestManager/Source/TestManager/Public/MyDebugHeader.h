#pragma once
#include "Misc/MessageDialog.h"
#include"Widgets/Notifications/SNotificationList.h"//֪ͨ�б�
#include"Framework/Notifications/NotificationManager.h"//֪ͨ��������




namespace MyDebugHeader {
	static void print(const FString& Message, const FColor& Color) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}
	static void pringLog(const FString& Message) {
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true) {
		if (bShowMsgAsWarning) {
			FText Msgtitle = FText::FromString(TEXT("Warning"));
			return FMessageDialog::Open(MsgType, FText::FromString(Message), &Msgtitle);//����ͷ
		}
		else {
			return FMessageDialog::Open(MsgType, FText::FromString(Message));

		}
	}

	static void showNotifyInfo(const FString& Message) {//��ʾ�����½ǵ�֪ͨ��Ϣ
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.0f;
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}
