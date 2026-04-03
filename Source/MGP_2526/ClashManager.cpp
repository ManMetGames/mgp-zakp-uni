#include "ClashManager.h"
#include "ClashEnemy.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

AClashManager::AClashManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AClashManager::BeginPlay()
{
    Super::BeginPlay();
}

void AClashManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bClashActive) return;

    // constant decay every second
    BarValue -= DecayRate * DeltaTime;

    // keep value between 0-1
    BarValue = FMath::Clamp(BarValue, 0.0f, 1.0f);

    // update UI
    UpdateWidget();

    // clash ends when bar reaches 0
    if (BarValue <= 0.0f)
    {
        EndClash(false);
    }
}

void AClashManager::StartClash(AClashEnemy* Enemy)
{
    if (bClashActive) return; // prevents another clash starting if there is one already

    CurrentEnemy = Enemy;

    if (!ClashWidgetClass) return;

    // create and show the widget
    ClashWidget = CreateWidget<UUserWidget>(GetWorld(), ClashWidgetClass);
    if (ClashWidget)
    {
        ClashWidget->AddToViewport();
    }

    // reset bar to middle and activate
    BarValue = 0.5f;
    bClashActive = true;

    // show first prompt
    SpawnNewPrompt();
}

void AClashManager::SpawnNewPrompt()
{
    // random possible keys that can show during clash
    TArray<FKey> PossibleKeys = {
        EKeys::Q, EKeys::W, EKeys::E,
        EKeys::R, EKeys::F, EKeys::G
    };

    // pick a random key
    int32 RandomIndex = FMath::RandRange(0, PossibleKeys.Num() - 1);
    CurrentPromptKey = PossibleKeys[RandomIndex];

    UpdateWidget();
}

void AClashManager::OnPlayerInput(FKey KeyPressed)
{
    if (!bClashActive) return;

    if (KeyPressed == CurrentPromptKey)
    {
        // if correct key — fill the bar by the fill amount
        BarValue += FillAmount;
        BarValue = FMath::Clamp(BarValue, 0.0f, 1.0f);

        // check win condition which is 1 or more
        if (BarValue >= 1.0f)
        {
            EndClash(true);
            return; // return won
        }

        // spawn next key promt
        SpawnNewPrompt();
    }
    // wrong key
    if (KeyPressed != CurrentPromptKey)
    {
        BarValue -= ReduceAmount;
        BarValue = FMath::Clamp(BarValue, 0.0f, 1.0f);
    }
}

void AClashManager::UpdateWidget()
{
    if (!ClashWidget) return;

    // update the progress bar
    if (UProgressBar* Bar = Cast<UProgressBar>(ClashWidget->GetWidgetFromName(TEXT("ClashBar"))))
    {
        Bar->SetPercent(BarValue);
    }

    // update the button prompt text
    if (UTextBlock* Text = Cast<UTextBlock>(ClashWidget->GetWidgetFromName(TEXT("ButtonPromptText"))))
    {
        Text->SetText(FText::FromString(CurrentPromptKey.GetDisplayName().ToString()));
    }
}

void AClashManager::EndClash(bool bPlayerWon)
{
    bClashActive = false;

    if (CurrentEnemy)
    {
        CurrentEnemy->bCanClash = true; //reset can clash back to true
        CurrentEnemy = nullptr;
    }


    // remove UI from screen when clash no longer active
    if (ClashWidget)
    {
        ClashWidget->RemoveFromParent();
        ClashWidget = nullptr;
    }

    if (bPlayerWon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player WON the clash!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Player LOST the clash."));
    }
}