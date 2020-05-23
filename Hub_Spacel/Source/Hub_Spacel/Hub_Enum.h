#pragma once

#include "CoreMinimal.h"
#include "Hub_Enum.generated.h"

UENUM(BlueprintType)
enum class ESubMachine : uint8
{
    /* Default and unset value */
    None,

    /* Head */
    Default UMETA(DisplayName = "Default"),
    Light UMETA(DisplayName = "Light")
};

UENUM(BlueprintType)
enum class EShell : uint8
{
    /* Default and unset value */
    None,

    /* Head */
    Default UMETA(DisplayName = "Default")
};