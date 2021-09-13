// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <typeinfo>

/**
 *
 */
struct HUB_SPACEL_API SClonable {
	virtual SClonable* cloner() const = 0;
	virtual ~SClonable() = default;
	virtual std::type_info const& get_type() const noexcept = 0;

protected:
	SClonable() = default;
	SClonable(const SClonable&) = default;
};