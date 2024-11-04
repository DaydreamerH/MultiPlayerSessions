// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"
class UButton;
class UMultiPlayerSessionsSubsystem;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = "Daydreamer");
	
protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCreateSeesion(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UFUNCTION()
	void HostButtonClick();
	UFUNCTION()
	void JoinButtonClick();

	void MenuTearDown();
	// hanle all online session functionality
	UMultiPlayerSessionsSubsystem* MultiPlayerSessionsSubsystem;

	int32 NumPublicConnections = 4;
	FString MatchType{TEXT("Daydreamer")};
};
