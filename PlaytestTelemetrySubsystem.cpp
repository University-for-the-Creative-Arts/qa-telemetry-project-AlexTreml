#include "PlaytestTelemetrySubsystem.h"            
#include "Misc/ConfigCacheIni.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

// Convert FJsonObject -> JSON string
static FString ToJson(const TSharedPtr<FJsonObject>& Obj)
{
    FString Out;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
    return Out;
}

void UPlaytestTelemetrySubsystem::LoadCfg()
{
    GConfig->GetString(TEXT("Playtest"), TEXT("SupabaseUrl"), SupabaseUrl, GGameIni);
    GConfig->GetString(TEXT("Playtest"), TEXT("SupabaseAnonKey"), AnonKey, GGameIni);
    GConfig->GetString(TEXT("Playtest"), TEXT("BuildTag"), BuildTag, GGameIni);

    // trim any stray whitespace
    SupabaseUrl = SupabaseUrl.TrimStartAndEnd();
    AnonKey     = AnonKey.TrimStartAndEnd().Replace(TEXT("\r"), TEXT("")).Replace(TEXT("\n"), TEXT(""));
    BuildTag    = BuildTag.TrimStartAndEnd();
}

void UPlaytestTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadCfg();
}

void UPlaytestTelemetrySubsystem::PostJSON(const FString& Endpoint, const FString& Body,
    TFunction<void(bool, const FString&)> Done)
{
    // Always post to Edge Functions
    const FString Url = FString::Printf(TEXT("%s/functions/v1/%s"), *SupabaseUrl, *Endpoint);

    UE_LOG(LogTemp, Warning, TEXT("### FUNCTIONS BUILD ACTIVE ###"));
    UE_LOG(LogTemp, Warning, TEXT("Posting to: %s"), *Url);
    UE_LOG(LogTemp, Warning, TEXT("Body: %s"), *Body);

    FHttpModule& Http = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetHeader(TEXT("apikey"), AnonKey);
    Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AnonKey));
    Req->SetHeader(TEXT("Prefer"), TEXT("return=representation"));
    Req->SetContentAsString(Body);

    Req->OnProcessRequestComplete().BindLambda(
        [Done](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
        {
            const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : -1;
            const bool bGood = bOk && Resp.IsValid() && Code >= 200 && Code < 300;
            if (!bGood)
            {
                const FString Payload = Resp.IsValid() ? Resp->GetContentAsString() : TEXT("<no response>");
                UE_LOG(LogTemp, Warning, TEXT("HTTP failed %d -> %s"), Code, *Payload);
            }
            Done(bGood, bGood ? Resp->GetContentAsString() : FString());
        });

    Req->ProcessRequest();
}

void UPlaytestTelemetrySubsystem::StartSession(bool bConsent, const FString& Device)
{
    TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
    Obj->SetBoolField(TEXT("consent"), bConsent);
    Obj->SetStringField(TEXT("device"), Device);
    Obj->SetStringField(TEXT("build"),  BuildTag);

    const FString Body = FString::Printf(TEXT("[%s]"), *ToJson(Obj));

    // Call the Edge Function by name
    PostJSON(TEXT("insert_session"), Body,
        [this](bool bOK, const FString& Payload)
        {
            if (!bOK)
            {
                UE_LOG(LogTemp, Warning, TEXT("StartSession: HTTP failed"));
                return;
            }

            // Parse [ { ... } ]
            TArray<TSharedPtr<FJsonValue>> Arr;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
            if (FJsonSerializer::Deserialize(Reader, Arr) && Arr.Num() > 0)
            {
                const TSharedPtr<FJsonObject> Row = Arr[0]->AsObject();
                if (Row.IsValid())
                {
                    SessionId = Row->GetStringField(TEXT("id"));
                    UE_LOG(LogTemp, Log, TEXT("Telemetry session started: %s"), *SessionId);
                }
            }
        });
}

void UPlaytestTelemetrySubsystem::LogEvent(const FString& Type, const TMap<FString,FString>& Fields)
{
    if (SessionId.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("LogEvent: Session not started yet"));
        return;
    }

    TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
    Obj->SetStringField(TEXT("session_id"), SessionId);
    Obj->SetStringField(TEXT("type"),       Type);

    TSharedPtr<FJsonObject> F = MakeShared<FJsonObject>();
    for (const auto& KV : Fields) { F->SetStringField(KV.Key, KV.Value); }
    Obj->SetObjectField(TEXT("fields"), F);

    const FString Body = FString::Printf(TEXT("[%s]"), *ToJson(Obj));

    // Function call
    PostJSON(TEXT("insert_event"), Body, [](bool, const FString&) {});
}

void UPlaytestTelemetrySubsystem::Tick(float DeltaTime)
{
    Frames++; Accum += DeltaTime; Since += DeltaTime;

    if (Since >= 1.f)
    {
        const float Avg = Accum / FMath::Max(1, Frames);
        const float FPS = (Avg > 0.f) ? 1.f / Avg : 0.f;

        TMap<FString,FString> F;
        F.Add(TEXT("avg_fps"), FString::SanitizeFloat(FPS));
        F.Add(TEXT("avg_frame_ms"), FString::SanitizeFloat(Avg * 1000.f));
        LogEvent(TEXT("perf_fps"), F);

        Frames = 0; Accum = 0.f; Since = 0.f;
    }
}
