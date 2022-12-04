// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstPersonCharacter.h"
#include "FirstPersonProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"



//////////////////////////////////////////////////////////////////////////
// AFirstPersonCharacter

AFirstPersonCharacter::AFirstPersonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a Fall Over Capsule Component
	FallOverCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("FallOverCapsule"));
	FallOverCapsuleComponent->SetupAttachment(GetCapsuleComponent());
	FallOverCapsuleComponent->InitCapsuleSize(55.f, 96.0f);
	FallOverCapsuleComponent->SetCollisionProfileName(TEXT("NoCollision"));

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

}

void AFirstPersonCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

}

//////////////////////////////////////////////////////////////////////////// Input

void AFirstPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AFirstPersonCharacter::OnPrimaryAction);

	// Fall over event
	PlayerInputComponent->BindAction("FallOver", IE_Pressed, this, &AFirstPersonCharacter::ToggleFallOver);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AFirstPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AFirstPersonCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AFirstPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AFirstPersonCharacter::LookUpAtRate);
}

void AFirstPersonCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
}

void AFirstPersonCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFirstPersonCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AFirstPersonCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFirstPersonCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFirstPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFirstPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

bool AFirstPersonCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFirstPersonCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFirstPersonCharacter::EndTouch);

		return true;
	}
	
	return false;
}

void AFirstPersonCharacter::ResetCharacterPostFall(){
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("ResetCharacterPostFall"));
	}
	FirstPersonCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	bool camsuccess = FirstPersonCameraComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	check(camsuccess)
	
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bLockToHmd = true;

	FallOverCapsuleComponent->SetWorldRotation(GetActorRotation());
	FallOverCapsuleComponent->SetWorldLocation(GetActorLocation());
	bool success = FallOverCapsuleComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);
	check(success)
	
	FallOverCapsuleComponent->SetCollisionProfileName(TEXT("NoCollision"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
}

void AFirstPersonCharacter::ToggleFallOver(){
	if (hasFallenOver) {
		GetBackUp();
	}
	/*else {
		FallOver();
	}*/

}

void AFirstPersonCharacter::OnHitWithBall(FVector Impulse, FVector Location)
{
	//Smoothly transition to our actor on begin play.
	FallOver();
	FallOverCapsuleComponent->AddImpulseAtLocation(Impulse, Location);
}

void AFirstPersonCharacter::FallOver(){
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("FallOver()"));
	}
	if (FallOverCapsuleComponent && !hasFallenOver)
	{
		hasFallenOver = true;
		FirstPersonCameraComponent->bUsePawnControlRotation = false;
		FirstPersonCameraComponent->bLockToHmd = false;
		FirstPersonCameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		bool camsuccess = FirstPersonCameraComponent->AttachToComponent(FallOverCapsuleComponent, FAttachmentTransformRules::KeepWorldTransform);
		check(camsuccess)

		FallOverCapsuleComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		FallOverCapsuleComponent->SetSimulatePhysics(true);
		FallOverCapsuleComponent->SetCollisionProfileName(TEXT("Ragdoll"));
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
	}
}

void AFirstPersonCharacter::GetBackUp(){
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("GetBackUp()"));
	}
	if (FallOverCapsuleComponent && hasFallenOver) {
		hasFallenOver = false;
		FallOverCapsuleComponent->SetSimulatePhysics(false);
		FallOverCapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
		SetActorLocation(FallOverCapsuleComponent->GetComponentLocation());

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		LatentInfo.ExecutionFunction = "ResetCharacterPostFall";
		LatentInfo.Linkage = 0;
		LatentInfo.UUID = 0;
		UKismetSystemLibrary::MoveComponentTo(FallOverCapsuleComponent, GetCapsuleComponent()->GetRelativeLocation(), GetCapsuleComponent()->GetRelativeRotation(), false, false, 0.5f, true, EMoveComponentAction::Type::Move, LatentInfo);
	}
}