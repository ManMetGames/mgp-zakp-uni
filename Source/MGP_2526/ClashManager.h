#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClashEnemy.h"
#include "ClashManager.generated.h"

class UUserWidget;

UCLASS()
class MGP_2526_API AClashManager : public AActor
{
    GENERATED_BODY()

public:
    AClashManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // the widget class to spawn — assign WBP_ClashUI in Blueprint
    UPROPERTY(EditAnywhere, Category = "Clash")
    TSubclassOf<UUserWidget> ClashWidgetClass;

    // how fast the bar decays per second
    UPROPERTY(EditAnywhere, Category = "Clash")
    float DecayRate = 0.1f;

    // how much a correct button press fills the bar
    UPROPERTY(EditAnywhere, Category = "Clash")
    float FillAmount = 0.2f;

    // how much a correct button press fills the bar
    UPROPERTY(EditAnywhere, Category = "Clash")
    float ReduceAmount = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Clash") // allow blueprint to display bar value
    float BarValue = 0.5f; // current bar value between 0 and 1

    UPROPERTY(BlueprintReadOnly, Category = "Clash")
    FKey CurrentPromptKey;// the current correct key the player needs to press

    // call this to kick off a clash
    void StartClash(AClashEnemy* Enemy);

    // call this when player presses a button during clash
    void OnPlayerInput(FKey KeyPressed);

private:
    // the actual widget instance
    UUserWidget* ClashWidget;

    AClashEnemy* CurrentEnemy;

    // whether a clash is currently happening
    bool bClashActive = false;

    // picks a new random button prompt
    void SpawnNewPrompt();

    // updates the bar and text on the widget
    void UpdateWidget();

    // ends the clash with win or lose
    void EndClash(bool bPlayerWon);
};