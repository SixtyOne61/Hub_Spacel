// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderActor.h"
#include "Util/Tag.h"
#include "Mesh/XmlInstancedStaticMeshComponent.h"

// Sets default values
ABuilderActor::ABuilderActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	createComponent(RedCubeComponent, TEXT("RedCube"));
	createComponent(WeaponComponent, TEXT("Weapon"));
	createComponent(ProtectionComponent, TEXT("Protection"));
	createComponent(EngineComponent, TEXT("Engine"));

	Tags.Add(Tags::Builder);
}

// Called when the game starts or when spawned
void ABuilderActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuilderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuilderActor::Export() const
{
	auto lb_call = [](UXmlInstancedStaticMeshComponent* _component)
	{
		if (_component != nullptr)
		{
			_component->Export();
		}
	};

	lb_call(WeaponComponent);
	lb_call(ProtectionComponent);
	lb_call(EngineComponent);
}

void ABuilderActor::Add(EBuilderType _type, FVector const& _location)
{
	if (auto* component = get(_type))
	{
		component->Add(_location);
	}
}

void ABuilderActor::Remove(EBuilderType _type, FVector const& _location)
{
	if (auto* component = get(_type))
	{
		component->Remove(_location);
	}
}

UXmlInstancedStaticMeshComponent* ABuilderActor::get(EBuilderType _type) const
{
	switch (_type)
	{
		case EBuilderType::Attack: return this->WeaponComponent;
		case EBuilderType::Protection: return this->ProtectionComponent;
		case EBuilderType::Engine: return this->EngineComponent;
	}

	return nullptr;
}