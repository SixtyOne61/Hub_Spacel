// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class HUB_SPACEL_API SpacelFactory
{
public:
	SpacelFactory() = delete;

	/* find class for T, only use on constructor */
	template<class T>
	static TSubclassOf<T> FindClass(TCHAR const* _path);
};

template<class T>
TSubclassOf<T> SpacelFactory::FindClass(TCHAR const* _path)
{
	ConstructorHelpers::FClassFinder<T> bpClass(_path);
	if (!ensure(bpClass.Class != nullptr)) return nullptr;

	return bpClass.Class;
}