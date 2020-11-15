// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"

namespace SimplyHttpRequest
{
    template<class ActorType, class ... Ts>
    static void processRequest(class FHttpModule* _module, UObject* _owner, void(ActorType::* _callback)(Ts...), FString&& _url, FString&& _verb, TOptional<TArray<FString>>&& _header, TOptional<FString>&& _content)
    {
        if (!ensure(_module != nullptr)) return;
        TSharedRef<IHttpRequest> request { _module->CreateRequest() };
        request->OnProcessRequestComplete().BindUObject(Cast<ActorType>(_owner), _callback);
        request->SetURL(_url);
        request->SetVerb(_verb);
        if (_header.IsSet())
        {
            for (int i = 0; i < _header.GetValue().Num()-1; i += 2)
            {
                request->SetHeader(_header.GetValue()[i], _header.GetValue()[i+1]);
            }
        }
        if (_content.IsSet())
        {
            request->SetContentAsString(_content.GetValue());
        }
        request->ProcessRequest();
    }

    static void processRequest(class FHttpModule* _module, FString&& _url, FString&& _verb, TOptional<TArray<FString>>&& _header, TOptional<FString>&& _content)
    {
        if (!ensure(_module != nullptr)) return;
        TSharedRef<IHttpRequest> request{ _module->CreateRequest() };
        request->SetURL(_url);
        request->SetVerb(_verb);
        if (_header.IsSet())
        {
            for (int i = 0; i < _header.GetValue().Num() - 1; i += 2)
            {
                request->SetHeader(_header.GetValue()[i], _header.GetValue()[i + 1]);
            }
        }
        if (_content.IsSet())
        {
            request->SetContentAsString(_content.GetValue());
        }
        request->ProcessRequest();
    }
}
