// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UMultiPlayerSessionsSubsystem::UMultiPlayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	if(const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterfaces = Subsystem->GetSessionInterface();
	}
}

void UMultiPlayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if(!SessionInterfaces.IsValid())
		return;

	auto ExistingSession = SessionInterfaces->GetNamedSession(NAME_GameSession);
	if(ExistingSession!=nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnection = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
		return;
	}

	CreateSessionCompleteDelegateHandle = SessionInterfaces->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	TObjectPtr<ULocalPlayer>LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterfaces->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterfaces->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		MultiPlayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiPlayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if(!SessionInterfaces.IsValid())
		return;

	FindSessionCompleteDelegateHandle = SessionInterfaces->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if(!SessionInterfaces->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterfaces->ClearOnFindSessionsCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiPlayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}

}

void UMultiPlayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if(!SessionInterfaces.IsValid())
	{
		MultiPlayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	
	JoinSessionCompleteDelegateHandle = SessionInterfaces->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	if(const TObjectPtr<UWorld> World = GetWorld())
	{
		const TObjectPtr<ULocalPlayer> LocalPlayer = World->GetFirstLocalPlayerFromController();
		if(!SessionInterfaces->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult))
		{
			SessionInterfaces->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
			MultiPlayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		}
	}
}

void UMultiPlayerSessionsSubsystem::DestroySession()
{
	if(!SessionInterfaces.IsValid())
	{
		MultiPlayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterfaces->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if(!SessionInterfaces->DestroySession(NAME_GameSession))
	{
		SessionInterfaces->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiPlayerOnDestroySessionComplete.Broadcast(false);
	}
	
}

void UMultiPlayerSessionsSubsystem::StartSession()
{
}

void UMultiPlayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterfaces)
	{
		SessionInterfaces->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	MultiPlayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiPlayerSessionsSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	
	if(!SessionInterfaces.IsValid())
	{
		SessionInterfaces->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
	}

	if(LastSessionSearch->SearchResults.Num()<=0)
	{
		MultiPlayerOnFindSessionComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	
	MultiPlayerOnFindSessionComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiPlayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if(SessionInterfaces)
		SessionInterfaces->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	MultiPlayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiPlayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterfaces)
	{
		SessionInterfaces->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		
	}
	if(bWasSuccessful&&bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnection, LastMatchType);
	}
	MultiPlayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiPlayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
