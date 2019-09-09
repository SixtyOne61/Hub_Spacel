// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldManager.h"
#include "EnvironmentManager.h"

// Sets default values
AWorldManager::AWorldManager()
	: m_nbChunck(10)
	, m_chunckSize(1000)
	, m_cubeSize(100)
	, BP_environment(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	spawnChunckEnvironment();
}

// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();
	
	spawnChunckEnvironment();
}

void AWorldManager::spawnChunckEnvironment()
{

}

