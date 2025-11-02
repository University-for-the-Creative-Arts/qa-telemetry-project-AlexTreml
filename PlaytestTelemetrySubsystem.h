#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h" // for FTickableGameObject
#include "PlaytestTelemetrySubsystem.generated.h"

/**
 * Supabase telemetry via Edge Functions.
 */
UCLASS()
class CAR_API UPlaytestTelemetrySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(UPlaytestTelemetrySubsystem, STATGROUP_Tickables);
    }
    virtual bool IsTickable() const override { return true; }

    // Plain C++ API
    void StartSession(bool bConsent, const FString& Device);
    void LogEvent(const FString& Type, const TMap<FString, FString>& Fields);

private:
    void LoadCfg();
    void PostJSON(const FString& Endpoint, const FString& Body,
                  TFunction<void(bool, const FString&)> Done);

    // Config
    FString SupabaseUrl;
    FString AnonKey;
    FString BuildTag;

    // State
    FString SessionId;

    // FPS accumulation
    int32 Frames = 0;
    float Accum = 0.f;
    float Since = 0.f;
};

