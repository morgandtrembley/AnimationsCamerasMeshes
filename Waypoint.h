#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waypoint.generated.h"

UCLASS()
class CAMERASANDMESHES_API AWaypoint : public AActor {
	GENERATED_BODY()
	
public:	
	AWaypoint();
	
	FVector DeltaHeight;
	float UpperBounce = 0.0f;
	float LowerBounce = 0.0f;

	UStaticMeshComponent* WaypointLower;
	UStaticMeshComponent* WaypointUpperIn;
	UStaticMeshComponent* WaypointUpperOut;

	virtual void Tick(float DeltaTime) override;
};