#include "ClashEnemy.h"

// Sets default values
AClashEnemy::AClashEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creating and setting the mesh
	EnemyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EnemyMesh"));
	RootComponent = EnemyMesh;

}

// Called when the game starts or when spawned
void AClashEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

