// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderActor.h"
#include "Util/Tag.h"
#include "Mesh/XmlInstancedStaticMeshComponent.h"

// Sets default values
ABuilderActor::ABuilderActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	auto lb_init = [&](UXmlInstancedStaticMeshComponent* _component, FName && _name)
	{
		_component = CreateDefaultSubobject<UXmlInstancedStaticMeshComponent>(_name);
		if (!ensure(_component != nullptr)) return;
		_component->SetRenderCustomDepth(true);
	};

	lb_init(WeaponComponent, TEXT("WeaponMeshComponent"));
	lb_init(ProtectionComponent, TEXT("ProtectionMeshComponent"));
	lb_init(EngineComponent, TEXT("EngineMeshComponent"));

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
		component->AddInstance(FTransform(_location));
	}
}

void ABuilderActor::Remove(EBuilderType _type, FVector const& _location)
{

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