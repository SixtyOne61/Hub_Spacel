// Fill out your copyright notice in the Description page of Project Settings.


#include "MatiereManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AMatiereManager::AMatiereManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MatiereMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Voxels"));
	RootComponent = MatiereMeshComponent;

	Tags.Add("Matiere");
}


