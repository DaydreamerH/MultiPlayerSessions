// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	
	TObjectPtr<UWorld> World = GetWorld();
	if(World)
	{
		if(const TObjectPtr<APlayerController> PlayerController = World->GetFirstPlayerController(); IsValid(PlayerController))
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);

			PlayerController->SetShowMouseCursor(true);
		}
	}
	if(const TObjectPtr<UGameInstance> GameInstance = GetGameInstance())
	{
		MultiPlayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionsSubsystem>();
	}

	if(MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSeesion);
		MultiPlayerSessionsSubsystem->MultiPlayerOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
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

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeConstruct();
}

void UMenu::OnCreateSeesion(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(const TObjectPtr<UWorld> World = GetWorld())
		{
			World->ServerTravel(PathToLobby);
		}
	}
	HostButton->SetIsEnabled(true);
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful) const
{
	if(!bWasSuccessful||SessionResults.Num()==0)
		JoinButton->SetIsEnabled(true);
	
	if(MultiPlayerSessionsSubsystem==nullptr)
		return;
	for(auto Result:SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"),SettingsValue);
		if(SettingsValue == MatchType)
		{
			MultiPlayerSessionsSubsystem->JoinSession(Result);
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result) const
{
	if(const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		const IOnlineSessionPtr SessionInterfaces = Subsystem->GetSessionInterface();
		if(SessionInterfaces.IsValid())
		{
			FString Address;
			SessionInterfaces->GetResolvedConnectString(NAME_GameSession, Address);

			if(const TObjectPtr<APlayerController> PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
		}
	}
	if(Result!=EOnJoinSessionCompleteResult::Success)
		JoinButton->SetIsEnabled(true);
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClick()
{
	HostButton->SetIsEnabled(false);
	if(MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClick()
{
	JoinButton->SetIsEnabled(false);
	if(MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	if(const TObjectPtr<UWorld> World = GetWorld())
	{
		TObjectPtr<APlayerController> PlayerController = World->GetFirstPlayerController();
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(false);
	}
}
