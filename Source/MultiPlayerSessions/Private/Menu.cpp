// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
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
			World->ServerTravel("/Game/_Game/Maps/Lobby?listen");
		}
	}
}

void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Enter List Find")));
	if(MultiPlayerSessionsSubsystem==nullptr)
		return;
	if(SessionResults.Num()==0)
	GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("0 results")));
	for(auto Result:SessionResults)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("List Results")));
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"),SettingsValue);
		if(SettingsValue == MatchType)
		{
			GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Start Join")));
			MultiPlayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if(const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterfaces = Subsystem->GetSessionInterface();
		if(!SessionInterfaces.IsValid())
		{
			FString Address;
			SessionInterfaces->GetResolvedConnectString(NAME_GameSession, Address);

			TObjectPtr<APlayerController> PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if(PlayerController)
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClick()
{
	if(MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClick()
{
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
