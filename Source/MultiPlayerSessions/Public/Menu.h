// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	void MenuSetup();
protected:
	virtual bool Initialize() override;
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UFUNCTION()
	void HostButtonClick();
	UFUNCTION()
	void JoinButtonClick();
	
	// hanle all online session functionality
	UMultiPlayerSessionsSubsystem* MultiPlayerSessionsSubsystem;
};
