// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreUserWidget.h"
#include "Util/SimplyUI.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/RankingUserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DataAsset/TeamColorDataAsset.h"

void UScoreUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ScoresVerticalBox = SimplyUI::initSafetyFromName<UUserWidget, UVerticalBox>(this, TEXT("VerticalBox_Score"));
}

void UScoreUserWidget::initScoreArray()
{
    if(this->ScoresVerticalBox == nullptr) return;

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        if(this->TeamColorDataAsset == nullptr) return;

        TArray<FScore> scores = spacelGameState->R_Scores;
        // search ranking
        scores.Sort([](FScore const& _s1, FScore const& _s2)
            {
                return _s1.Score > _s2.Score;
            });

        for (FScore const& score : scores)
        {
            URankingUserWidget* teamScore = CreateWidget<URankingUserWidget, UVerticalBox>(this->ScoresVerticalBox, this->RankingWidgetClass, *score.Team);
            teamScore->SetTeam(score.Team, this->TeamColorDataAsset->GetColor<FSlateColor>(score.Team), score.Score);
            this->ScoresVerticalBox->AddChildToVerticalBox(teamScore);
        }
    }
}

void UScoreUserWidget::updateScore()
{
    if (this->ScoresVerticalBox == nullptr) return;

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        if (this->TeamColorDataAsset == nullptr) return;

        TArray<FScore> scores = spacelGameState->R_Scores;
        // search ranking
        scores.Sort([](FScore const& _s1, FScore const& _s2)
            {
                return _s1.Score > _s2.Score;
            });

        int32 index = 0;
        for (FScore const& score : scores)
        {
            if (URankingUserWidget* rankingUserWidget = Cast<URankingUserWidget>(this->ScoresVerticalBox->GetChildAt(index)))
            {
                rankingUserWidget->SetTeam(score.Team, this->TeamColorDataAsset->GetColor<FSlateColor>(score.Team), score.Score);
                ++index;
            }
        }
    }
}