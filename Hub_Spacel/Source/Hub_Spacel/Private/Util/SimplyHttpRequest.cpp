// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplyHttpRequest.h"
#include "Http.h"

void SimplyHttpRequest::processRequest(FHttpModule* _module, FString&& _url, FString&& _verb)
{
    TSharedRef<IHttpRequest> request { _module->CreateRequest() };
    //request->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::onExchangeCodeForTokensResponseReceived);
    request->SetURL(_url);
    request->SetVerb(_verb);
    //request->SetHeader("Content-Type", "application/json");
    //request->SetContentAsString(requestBody);
    request->ProcessRequest();
}
