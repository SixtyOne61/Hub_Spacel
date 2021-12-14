// Fill out your copyright notice in the Description page of Project Settings.


#include "ScoreDataAsset.h"

void UScoreDataAsset::FillInfo(EScoreType _type, FScoreData& _out) const
{
	for (auto const& score : Scores)
	{
		if (score.Type == _type)
		{
			_out = score;
		}
	}
}