// Copyright Epic Games, Inc. All Rights Reserved.

#include "CamerasAndMeshesCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

ACamerasAndMeshesCharacter::ACamerasAndMeshesCharacter() {
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(0.0f, 0.0f, 0.0f));
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	// first person camera set up
	FirstPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonSpringArm"));
	FirstPersonSpringArm->SetupAttachment(GetMesh(), TEXT("Head"));	// attach to player head
	FirstPersonSpringArm->bUsePawnControlRotation = true;
	FirstPersonSpringArm->SetRelativeLocationAndRotation(FVector(12.0f, 5.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
	FirstPersonSpringArm->TargetArmLength = -20.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(FirstPersonSpringArm, USpringArmComponent::SocketName);
	FirstPersonCamera->bUsePawnControlRotation = false;

	// main map camera set up
	MainMapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MainMapSpringArm"));
	MainMapSpringArm->SetupAttachment(RootComponent);
	MainMapSpringArm->bUsePawnControlRotation = true;
	MainMapSpringArm->TargetArmLength = 0.0f;
	MainMapSpringArm->SetUsingAbsoluteLocation(true);
	MainMapSpringArm->SetRelativeLocationAndRotation(FVector(MAIN_CAM_LOCATION), FRotator(0.0f, 0.0f, 0.0f));

	MainMapCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainMapCamera"));
	MainMapCamera->SetupAttachment(MainMapSpringArm, USpringArmComponent::SocketName);
	MainMapCamera->bUsePawnControlRotation = false;
	MainMapCamera->SetUsingAbsoluteRotation(true);
	MainMapCamera->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 0.0f), FRotator(-90.0f, 0.0f, 0.0f));

	// mini map camera set up
	MiniMapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MiniMapSpringArm"));
	MiniMapSpringArm->SetupAttachment(RootComponent);
	MiniMapSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 300.0f), FRotator(-90.0f, 0.0f, 0.0f));
	MiniMapSpringArm->TargetArmLength = 300.0f;

	
	// waypoint arrow set up 
	WaypointArrowSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("WaypointArrowSpringArm"));
	WaypointArrowSpringArm->SetupAttachment(RootComponent);
	WaypointArrowSpringArm->TargetArmLength = -100.0f;
	WaypointArrowSpringArm->SetUsingAbsoluteRotation(true);

	WaypointArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaypointArrow"));
	WaypointArrow->SetupAttachment(WaypointArrowSpringArm);
	WaypointArrow->SetRelativeScale3D(FVector(0.125f, 0.125f, 0.125f));
	WaypointArrow->SetHiddenInGame(true);
	WaypointArrow->SetCastShadow(false);
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	// blueprint classes
	static ConstructorHelpers::FClassFinder<UUserWidget> InGameUIBPClass(TEXT("/Game/1MyContent/Blueprints/MiniMap"));
	MiniMapClass = InGameUIBPClass.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> MainMapBPClass(TEXT("/Game/1MyContent/Blueprints/MainMap"));
	MainMapClass = MainMapBPClass.Class;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACamerasAndMeshesCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACamerasAndMeshesCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACamerasAndMeshesCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACamerasAndMeshesCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACamerasAndMeshesCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACamerasAndMeshesCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACamerasAndMeshesCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACamerasAndMeshesCharacter::OnResetVR);

	// scroll functionality for player cameras
	PlayerInputComponent->BindAction("ScrollIn", IE_Pressed, this, &ACamerasAndMeshesCharacter::OnScrollIn);
	PlayerInputComponent->BindAction("ScrollOut", IE_Pressed, this, &ACamerasAndMeshesCharacter::OnScrollOut);
	
	// open map
	PlayerInputComponent->BindAction("Map", IE_Pressed, this, &ACamerasAndMeshesCharacter::ShowHideMap);
	
	// waypoint creation and destruction
	PlayerInputComponent->BindAction("SetWaypoint", IE_Pressed, this, &ACamerasAndMeshesCharacter::LeftClick);
	PlayerInputComponent->BindAction("DeleteWaypoint", IE_Pressed, this, &ACamerasAndMeshesCharacter::RightClick);

	// sprint functionality
	PlayerInputComponent->BindAction("ToggleSprint", IE_Pressed, this, &ACamerasAndMeshesCharacter::ToggleSprintOn);
	PlayerInputComponent->BindAction("ToggleSprint", IE_Released, this, &ACamerasAndMeshesCharacter::ToggleSprintOff);
}

void ACamerasAndMeshesCharacter::ToggleSprintOn() { Sprint = true; }
void ACamerasAndMeshesCharacter::ToggleSprintOff() { Sprint = false; }

// right click heavy attacks when controlling player and can delete a waypoint when main map is open
void ACamerasAndMeshesCharacter::RightClick() {
	// if map is open
	if (MainMapCamera->IsActive()) {
		FVector WorldLocation, WorldDirection;
		FVector Start = FVector(MAIN_CAM_LOCATION);

		// deproject cursor location from map view to level location with respect to main map camera
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

		// length of line trace
		FVector End = WorldLocation + WorldDirection * 100000;

		// line trace from main map camera to level location of cursor
		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
		float X = FMath::Abs(Hit.Location.X - Waypoint->GetActorLocation().X);
		float Y = FMath::Abs(Hit.Location.Y - Waypoint->GetActorLocation().Y);

		// if there is a waypoint in the general area of cursor, remove it and hide waypoint arrow
		if (IsValid(Waypoint) && X < 200 && Y < 200) {
			Waypoint->Destroy();
			WaypointArrow->SetHiddenInGame(true);
		}
	}
	else {
		// call blueprint function to play attack animation
		HeavyAttack();
	}

}

// left click light attacks when controlling player and sets a waypoint when main map is open
void ACamerasAndMeshesCharacter::LeftClick() {
	// if map is open
	if (MainMapCamera->IsActive()) {

		// if waypoint already exists, destroy it
		if (IsValid(Waypoint)) {
			Waypoint->Destroy();
		}

		FVector WorldLocation, WorldDirection;
		FVector Start = FVector(MAIN_CAM_LOCATION);

		// deproject cursor location from map view to level location with respect to main map camera
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

		// length of line trace
		FVector End = WorldLocation + WorldDirection * 100000;

		// line trace from main map camera to level location of cursor
		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Orange, true, 2.0f);
		//DrawDebugPoint(GetWorld(), Hit.Location,10, FColor::Orange, true);


		// spawn waypoint
		Waypoint = (AWaypoint*) GetWorld()->SpawnActor<AWaypoint>(Hit.Location, FRotator(0.0f,0.0f,0.0f));

		// set waypoint arrow as visible
		WaypointArrow->SetHiddenInGame(false);
	}
	else {
		// call blueprint function to play attack animation
		LightAttack();
	}
	
}

void ACamerasAndMeshesCharacter::ShowHideMap() {
	// toggle main map on or off
	if (MainMapCamera->IsActive()) {

		// return to last used camera
		if (POV) {
			FirstPersonCamera->SetActive(true);
		}
		else {
			FollowCamera->SetActive(true);
		}

		// deactivate main map camera
		MainMapCamera->SetActive(false);

		// reset control settings for character movement
		MyController->bShowMouseCursor = false;
		MyController->bEnableClickEvents = false;
		MyController->SetIgnoreMoveInput(false);
		MyController->SetIgnoreLookInput(false);

		// add minimap widget to viewport and hide main map overlay widget
		wMiniMap->AddToViewport();
		wMainMap->RemoveFromViewport();
	}
	else {
		// activate main map camera
		MainMapCamera->SetActive(true);

		// deactivate last used camera
		if (POV) {
			FirstPersonCamera->SetActive(false);
		}
		else {
			FollowCamera->SetActive(false);
		}

		// set controller settings to interact with map and disable player movement
		MyController->bShowMouseCursor = true;
		MyController->bEnableClickEvents = true;
		MyController->SetIgnoreMoveInput(true);
		MyController->SetIgnoreLookInput(true);

		// hide minimap widget from viewport and show main map overlay
		wMiniMap->RemoveFromViewport();
		wMainMap->AddToViewport();
	}
}

void ACamerasAndMeshesCharacter::OnScrollOut() {
	// check that player cameras are active
	if (!MainMapCamera->IsActive()) {
		// if camera is at max distance - do nothing
		if (CameraBoom->TargetArmLength <= MAX_CAM_DIST) {
			// if first person camera is active switch to thrid person view
			if (POV) {
				// switch to third person camera and corresponding settings
				FollowCamera->SetActive(true);
				FirstPersonCamera->SetActive(false);
				bUseControllerRotationYaw = false;

				// change waypoint arrow distance from character for the 3rd person camera
				WaypointArrowSpringArm->TargetArmLength = -100.0f;

				POV = THIRD_PERSON;
			}
			else {
				// move camera 1 step backward
				CameraBoom->TargetArmLength += CAM_STEP;
			}
		}
	}
}

void ACamerasAndMeshesCharacter::OnScrollIn() {
	// check that player cameras are active
	if (!MainMapCamera->IsActive()) {
		// if First Person POV - do nothing
		if (!POV) {
			// switch to first person view
			if (CameraBoom->TargetArmLength <= MIN_CAM_DIST) {
				// switch to first person camera and corresponding settings
				FirstPersonCamera->SetActive(true);
				FollowCamera->SetActive(false);
				bUseControllerRotationYaw = true;

				// Move waypoint arrow further from character so it's easily visible from 1st person view
				WaypointArrowSpringArm->TargetArmLength = -200.0f;

				POV = FIRST_PERSON;
			}
			else {
				// move camera 1 step closer to player character
				CameraBoom->TargetArmLength -= CAM_STEP;
			}
		}
	}
}

void ACamerasAndMeshesCharacter::OnResetVR() {
	// If CamerasAndMeshes is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in CamerasAndMeshes.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACamerasAndMeshesCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location) {
		Jump();
}

void ACamerasAndMeshesCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location) {
		StopJumping();
}

void ACamerasAndMeshesCharacter::TurnAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACamerasAndMeshesCharacter::LookUpAtRate(float Rate) {
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACamerasAndMeshesCharacter::MoveForward(float Value) {
	if ((Controller != nullptr) && (Value != 0.0f)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// sprint functionality
		if (!Sprint) {
			Value = Value / 2;
		}

		AddMovementInput(Direction, Value);
	}
}

void ACamerasAndMeshesCharacter::MoveRight(float Value) {
	if ( (Controller != nullptr) && (Value != 0.0f) ) {
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// sprint functionality
		if (!Sprint) {
			Value = Value / 2;
		}

		AddMovementInput(Direction, Value);
	}
}

void ACamerasAndMeshesCharacter::BeginPlay() {
	Super::BeginPlay();

	// start on 3rd person camera
	FollowCamera->SetActive(true);
	FirstPersonCamera->SetActive(false);
	MainMapCamera->SetActive(false);

	MyController = Cast<APlayerController>(GetController());

	// widget creation
	wMainMap = CreateWidget<UMainMapWidget>(GetWorld(), MainMapClass);
	wMiniMap = CreateWidget<UMiniMapWidget>(GetWorld(), MiniMapClass);

	// add minimap to viewport since we are in 3rd person
	wMiniMap->AddToViewport();
	
}

void ACamerasAndMeshesCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	// if a waypoint exists in the level
	if (IsValid(Waypoint)) {

		// wapoint arrow point direction
		WaypointDirection = Waypoint->GetActorLocation() - WaypointArrowSpringArm->GetComponentLocation();
		WaypointLookAtDirection = FRotationMatrix::MakeFromX(WaypointDirection).Rotator();

		// zero out directions we don't want to change
		WaypointLookAtDirection.Pitch = 0.0f;
		WaypointLookAtDirection.Roll = 0.0f;

		// update waypoint arrow point direction
		WaypointArrowSpringArm->SetWorldRotation(WaypointLookAtDirection);
	}
}