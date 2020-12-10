// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ASpacelGameState::OnRep_StateGame()
{
    switch (this->RU_GameState)
    {
    case (uint8)EGameState::Prepare:
        OnStartPrepareDelegate.Broadcast();
        break;

    case (uint8)EGameState::InGame:
        OnStartGameDelegate.Broadcast();
        break;

    default:
        break;
    }
}

FString ASpacelGameState::GetBestTeam() const
{
    TOptional<int32> val {};
    FString teamName {};
    for (auto const& score : m_scores)
    {
        if (!val.IsSet() || score.Value > val.GetValue())
        {
            val = score.Value;
            teamName = score.Key;
        }
    }

    return teamName;
}

void ASpacelGameState::AddScore(FString const& _team, int32 _val)
{
    m_scores[_team] += _val;
}

void ASpacelGameState::AttributePlayersLocation()
{
    for (APlayerState* playerState : this->PlayerArray)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            FString const& teamName = spacelPlayerState->Team;
            // register team for scoring
            if (!m_scores.Contains(teamName))
            {
                m_scores.Add(teamName);
            }
            
            FTeamLocation* teamLocation = this->TeamsLocation.FindByPredicate([&teamName](auto const& _obj) {
                return _obj.Name == teamName;
                });

            if (teamLocation && teamLocation->Transforms.Num() > 0)
            {
                FTransform const& transform = teamLocation->Transforms[0];
                if (APawn* pawn = spacelPlayerState->GetPawn())
                {
                    spacelPlayerState->PlayerStartTransform = transform;
                    pawn->SetActorLocationAndRotation(transform.GetLocation(), transform.GetRotation());
                    teamLocation->Transforms.RemoveAt(0);
                }
            }
        }
    }
}

void ASpacelGameState::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> actors;
    UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), APlayerStart::StaticClass(), actors);
    for (AActor* actor : actors)
    {
        if(actor == nullptr) continue;

        if (actor->Tags.Num() > 0)
        {
            FName const& teamName = actor->Tags[0];
            if (FTeamLocation* teamLocation = this->TeamsLocation.FindByPredicate([&teamName](auto const& _obj) {
                return _obj.Name == teamName.ToString();
                }))
            {
                teamLocation->Transforms.Add(actor->GetTransform());
            }
        }
    }
}

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
}