// Fill out your copyright notice in the Description page of Project Settings.


#include "LightSubMachine.h"
#include "Components/StaticMeshComponent.h"

ALightSubMachine::ALightSubMachine()
    : ADefaultSubMachine()
{
    auto lb_initSpawner = [&](FName&& _name, UStaticMeshComponent*& _comp)
    {
        _comp = CreateDefaultSubobject<UStaticMeshComponent>(_name);
        _comp->SetEnableGravity(false);
        _comp->SetGenerateOverlapEvents(false);
        _comp->SetCollisionProfileName("NoCollision");
        _comp->SetupAttachment(RootComponent);
        m_bulletSpawners.Add(_comp);
    };

    lb_initSpawner(TEXT("BulletSpawner1"), BulletSpawner1);
    lb_initSpawner(TEXT("BulletSpawner2"), BulletSpawner2);
    lb_initSpawner(TEXT("BulletSpawner3"), BulletSpawner3);
    lb_initSpawner(TEXT("BulletSpawner4"), BulletSpawner4);
    lb_initSpawner(TEXT("BulletSpawner5"), BulletSpawner5);
    lb_initSpawner(TEXT("BulletSpawner6"), BulletSpawner6);
    lb_initSpawner(TEXT("BulletSpawner7"), BulletSpawner7);
}
