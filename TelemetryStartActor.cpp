#include "TelemetryStartActor.h"
#include "Engine/GameInstance.h"
#include "PlaytestTelemetrySubsystem.h"

ATelemetryStartActor::ATelemetryStartActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATelemetryStartActor::BeginPlay()
{
    Super::BeginPlay();

    if (!bAutoStart) return;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UPlaytestTelemetrySubsystem* Telemetry = GI->GetSubsystem<UPlaytestTelemetrySubsystem>())
        {
            UE_LOG(LogTemp, Warning, TEXT("TelemetryStartActor: StartSession fired (%s)"),
                *DeviceLabel);
            Telemetry->StartSession(true, DeviceLabel);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("TelemetryStartActor: Telemetry subsystem not found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TelemetryStartActor: No GameInstance"));
    }
}
