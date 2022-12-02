// Copyright Epic Games, Inc. All Rights Reserved.

#include "Wraith.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "WraithAIController.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AWraith

AWraith::AWraith()
{
	hasFallenOver = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWraith::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AWraith::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AWraith::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AWraith::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AWraith::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AWraith::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AWraith::TouchStopped);
}

void AWraith::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AWraith::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AWraith::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AWraith::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AWraith::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AWraith::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AWraith::OnHitWithBall(FVector Impulse, FVector Location){

	if (!hasFallenOver) {
		hasFallenOver = true;
		USkeletalMeshComponent* mesh = GetMesh();
		UCapsuleComponent* cap = GetCapsuleComponent();
		UCharacterMovementComponent* movement = GetCharacterMovement();
		cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		cap->SetCollisionResponseToAllChannels(ECR_Ignore);
		mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		mesh->SetSimulatePhysics(true);
		mesh->WakeAllRigidBodies();
		mesh->bBlendPhysics = true;
		mesh->HideBoneByName("weapon_r", EPhysBodyOp::PBO_None);
		movement->StopMovementImmediately();
		movement->DisableMovement();
		movement->SetComponentTickEnabled(false);
		AController* controller = GetController();
		AWraithAIController* wc = Cast<AWraithAIController>(controller);
		if (wc) {
			wc->StopBehaviourTree();
		}

		GetWorldTimerManager().SetTimer(GetUpTimerHandle, this, &AWraith::GetBackUp, 1.0f, false, 2.0f);
	}

	GetMesh()->AddImpulseAtLocation(Impulse, Location);
}

void AWraith::GetBackUp(){
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Wraith::GetBackUp"));
	if (hasFallenOver) {
		USkeletalMeshComponent* mesh = GetMesh();
		UCapsuleComponent* cap = GetCapsuleComponent();
		UCharacterMovementComponent* movement = GetCharacterMovement();

		SetActorLocation(mesh->GetComponentLocation());
		cap->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		cap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		cap->SetCollisionProfileName("Pawn");

		mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		mesh->SetSimulatePhysics(false);
		mesh->bBlendPhysics = false;
		mesh->UnHideBoneByName("weapon_r");

		//movement->DisableMovement(); //might be an issue. We'll see.
		movement->SetComponentTickEnabled(true);

		AController* controller = GetController();
		AWraithAIController* wc = Cast<AWraithAIController>(controller);
		if (wc) {
			wc->RestartBehaviourTree();
		}
		
		hasFallenOver = false;
		GetWorldTimerManager().ClearTimer(GetUpTimerHandle);
	}
}