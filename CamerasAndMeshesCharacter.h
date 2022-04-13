#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MinimapWidget.h"
#include "MainMapWidget.h"
#include "Waypoint.h"
#include "CamerasAndMeshesCharacter.generated.h"

#define THIRD_PERSON 0
#define FIRST_PERSON 1
#define MAX_CAM_DIST 1000.0f
#define MIN_CAM_DIST 300.0f
#define CAM_STEP 20.0f
#define MAIN_CAM_LOCATION 6000.0f, 10000.0f, 30000.0f

UCLASS(config=Game)
class ACamerasAndMeshesCharacter : public ACharacter {
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


	// first person camera boom
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* FirstPersonSpringArm;

	// first person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCamera;


	// main map camera boom
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* MainMapSpringArm;

	// main map camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* MainMapCamera;


	// mini map camera boom
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* MiniMapSpringArm;


	// waypoint arrow spring arm (points the player to active waypoint)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* WaypointArrowSpringArm;

	// waypoint arrow visual mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WaypointArrow;

public:
	ACamerasAndMeshesCharacter();

	UFUNCTION(BlueprintImplementableEvent)
	void LightAttack();

	UFUNCTION(BlueprintImplementableEvent)
	void HeavyAttack();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	APlayerController* MyController;

	TSubclassOf<UUserWidget> MiniMapClass;
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UMiniMapWidget* wMiniMap;

	TSubclassOf<UUserWidget> MainMapClass;
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UMainMapWidget* wMainMap;

	bool POV;
	bool Sprint;
	
	FVector WaypointDirection;
	FRotator WaypointLookAtDirection;

	AWaypoint* Waypoint;

protected:
	void ToggleSprintOn();
	void ToggleSprintOff();

	void LeftClick();
	void RightClick();

	void ShowHideMap();

	void OnScrollIn();
	void OnScrollOut();

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	virtual void Tick(float DeltaTime);

	virtual void BeginPlay() override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns FirstPersonSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetFirstPersonSpringArm() const { return FirstPersonSpringArm; }
	/** Returns FirstPersonCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
	/** Returns MainMapSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetMainMapSpringArm() const { return MainMapSpringArm; }
	/** Returns MainMapCamera subobject **/
	FORCEINLINE class UCameraComponent* GetMainMapCamera() const { return MainMapCamera; }
	/** Returns MinimapSpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetMinimapSpringArm() const { return MiniMapSpringArm; }
};

