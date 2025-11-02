#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TelemetryStartActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class CAR_API ATelemetryStartActor : public AActor
{
    GENERATED_BODY()

public:
    ATelemetryStartActor();

    // Auto-fire StartSession on BeginPlay
    UPROPERTY(EditAnywhere, Category = "Playtest")
    bool bAutoStart = true;

    // Sent to Supabase in StartSession
    UPROPERTY(EditAnywhere, Category = "Playtest")
    FString DeviceLabel = TEXT("Windows Desktop");

protected:
    virtual void BeginPlay() override;
};
