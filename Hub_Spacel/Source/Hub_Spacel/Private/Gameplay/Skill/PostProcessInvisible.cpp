// Fill out your copyright notice in the Description page of Project Settings.


#include "PostProcessInvisible.h"

// Sets default values
APostProcessInvisible::APostProcessInvisible()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APostProcessInvisible::OnRemoveEffect(EEffect _effect)
{
	if (_effect == EEffect::MetaFormSupport || _effect == EEffect::Killed)
	{
		this->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		this->Destroy();
	}
}


