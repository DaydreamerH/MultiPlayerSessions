// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "Components/Button.h"

void UMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	
	TObjectPtr<UWorld> World = GetWorld();
	if(World)
	{
		TObjectPtr<APlayerController> PlayerController = World->GetFirstPlayerController();
		if(IsValid(PlayerController))
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);

			PlayerController->SetShowMouseCursor(true);
		}
	}
	TObjectPtr<UGameInstance> GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiPlayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionsSubsystem>();
	}
	if(MultiPlayerSessionsSubsystem)
	{
		// Wait to complete
		MultiPlayerSessionsSubsystem->CreateSession(4, FString("MyPlugin"));
	}
}

bool UMenu::Initialize()
{
	if(!Super::Initialize())return false;

	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClick);
	}
	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClick);
	}
	
	return true;
}

void UMenu::HostButtonClick()
{
}

void UMenu::JoinButtonClick()
{
}
