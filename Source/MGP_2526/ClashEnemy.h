#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClashEnemy.generated.h"

UCLASS()
class MGP_2526_API AClashEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AClashEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
    // Enemy mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* EnemyMesh;

    // Range for initiating the clash
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clash")
    float InteractionRange = 300.0f;

    // Whether this enemy is available to clash
    UPROPERTY(BlueprintReadOnly, Category = "Clash")
    bool bCanClash = true;

};
