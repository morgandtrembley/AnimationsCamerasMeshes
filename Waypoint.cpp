#include "Waypoint.h"

AWaypoint::AWaypoint() {
	PrimaryActorTick.bCanEverTick = true;

	// visual mesh for lower part of waypoint
	ConstructorHelpers::FObjectFinder<UStaticMesh> BotMesh(TEXT("/Game/1MyContent/Meshes/WaypointBot.WaypointBot"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> BotMaterial(TEXT("/Game/1MyContent/Materials/WaypointBotMaterialInstance.WaypointBotMaterialInstance"));
	WaypointLower = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Waypoint Lower"));
	WaypointLower->SetStaticMesh(BotMesh.Object);
	WaypointLower->SetupAttachment(RootComponent);
	WaypointLower->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	WaypointLower->SetRelativeLocation(FVector(0.0f,0.0f,150.0f));
	WaypointLower->SetMaterial(0, BotMaterial.Object);

	// visual mesh for upper interior part of waypoint
	ConstructorHelpers::FObjectFinder<UStaticMesh> TopMeshIn(TEXT("/Game/1MyContent/Meshes/WaypointInside.WaypointInside"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> TopMaterialIn(TEXT("/Game/1MyContent/Materials/WaypointTopMaterialInstance.WaypointTopMaterialInstance"));
	WaypointUpperIn = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Waypoint Upper In"));
	WaypointUpperIn->SetStaticMesh(TopMeshIn.Object);
	WaypointUpperIn->SetupAttachment(WaypointLower);
	WaypointUpperIn->SetRelativeLocation(FVector(0.0f, 0.0f, 850.0f));
	WaypointUpperIn->SetRelativeScale3D(FVector(2, 2, 2));
	WaypointUpperIn->SetMaterial(0, TopMaterialIn.Object);

	// visual mesh for upper exterior part of waypoint
	ConstructorHelpers::FObjectFinder<UStaticMesh> TopMesh(TEXT("/Game/1MyContent/Meshes/WaypointOutside.WaypointOutside"));
	ConstructorHelpers::FObjectFinder<UMaterialInstance> TopMaterial(TEXT("/Game/1MyContent/Materials/WaypointBotMaterialInstance.WaypointBotMaterialInstance"));
	WaypointUpperOut = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Waypoint Upper Out"));
	WaypointUpperOut->SetStaticMesh(TopMesh.Object);
	WaypointUpperOut->SetupAttachment(WaypointUpperIn);
	WaypointUpperOut->SetMaterial(0, TopMaterial.Object);
}

void AWaypoint::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	// bouncing and rotation for upper part of waypoint
	DeltaHeight.Z = 0.5*FMath::Sin(UpperBounce);
	FVector UpperLocation = WaypointUpperIn->GetRelativeLocation();
	WaypointUpperIn->SetRelativeLocation(UpperLocation + DeltaHeight);
	WaypointUpperIn->AddLocalRotation(FRotator(0.0f, 0.5f, 0.0f));

	// bouncing for lower part of waypoint
	FVector LowerLocation = WaypointLower->GetRelativeLocation();
	DeltaHeight.Z = DeltaHeight.Z = 0.05 * FMath::Sin(LowerBounce);
	WaypointLower->SetRelativeLocation(LowerLocation + DeltaHeight);

	// bounce speed modifiers
	UpperBounce += 0.02;
	LowerBounce += 0.01;
}

