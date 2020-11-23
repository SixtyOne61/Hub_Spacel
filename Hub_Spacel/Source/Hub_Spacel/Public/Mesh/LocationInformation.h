#pragma once
#include "Enum/SpacelEnum.h"
#include "LocationInformation.generated.h"

USTRUCT(BlueprintType)
struct FLocationInformation
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float NoiseValue { };

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool Used { false };

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int Index { -1 };

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Location { };

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EFace Mask = EFace::None;

    /* only use on editor */
    TMap<EFace, FLocationInformation*> m_neighboor = { {EFace::Back, nullptr }, {EFace::Front, nullptr },
                                            {EFace::Bot, nullptr }, {EFace::Top, nullptr },
                                            {EFace::Right, nullptr }, {EFace::Left, nullptr } };

    TArray<int> m_neighboorById { -1, -1, -1, -1, -1, -1 };

    bool isValid() const;
    int getNeighboorId(EFace const& _f) const;
    void setNeighboorId(EFace const& _f, int _id = -1);

    FString getXml() const;
};

inline bool FLocationInformation::isValid() const
{
    return this->NoiseValue > .75f && !this->Used;
}

inline int FLocationInformation::getNeighboorId(EFace const& _f) const
{
    switch (_f)
    {
        case EFace::Back:
            return m_neighboorById[0];
        case EFace::Front:
            return m_neighboorById[1];
        case EFace::Bot:
            return m_neighboorById[2];
        case EFace::Top:
            return m_neighboorById[3];
        case EFace::Right:
            return m_neighboorById[4];
        case EFace::Left:
            return m_neighboorById[5];
    }

    return -1;
}

inline void FLocationInformation::setNeighboorId(EFace const& _f, int _id /*= -1*/)
{
    switch (_f)
    {
    case EFace::Back:
        m_neighboorById[0] = _id;
        break;
    case EFace::Front:
        m_neighboorById[1] = _id;
        break;
    case EFace::Bot:
        m_neighboorById[2] = _id;
        break;
    case EFace::Top:
        m_neighboorById[3] = _id;
        break;
    case EFace::Right:
        m_neighboorById[4] = _id;
        break;
    case EFace::Left:
        m_neighboorById[5] = _id;
        break;
    }
}