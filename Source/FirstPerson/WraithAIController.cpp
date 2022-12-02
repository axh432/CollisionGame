// Fill out your copyright notice in the Description page of Project Settings.


#include "WraithAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FirstPersonCharacter.h"

void AWraithAIController::BeginPlay() {
	check(GEngine);
	//MoveCloseToEnemy();
	RunBehaviorTree(BehaviourTree);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Called super"));
	Super::BeginPlay();
}

void AWraithAIController::FindAnEnemy(){}

void AWraithAIController::MoveCloseToEnemy(){
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("WraithController::MoveCloseToEnemy"));

	TArray<AActor*> MatchingActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFirstPersonCharacter::StaticClass(), MatchingActors);

	if (!MatchingActors.IsEmpty()) {
		FString name = MatchingActors[0]->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("MovingToActor: " + name));

		FVector Destination = MatchingActors[0]->GetActorLocation();
		EPathFollowingRequestResult::Type res = MoveToLocation(Destination, 500.0f, true, true, true);

		switch (res) {
			case EPathFollowingRequestResult::Type::Failed:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("MoveToLocation: Failed"));
					break;
			case EPathFollowingRequestResult::Type::AlreadyAtGoal:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("MoveToLocation: AlreadyAtGoal"));
					break;
			case EPathFollowingRequestResult::Type::RequestSuccessful:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("MoveToLocation: RequestSuccessful"));
					break;
			default:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("MoveToLocation: default"));
		}

	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("GetAllActorsOfClass failed"));
	}
}

void AWraithAIController::AttackEnemy(){}

void AWraithAIController::StopBehaviourTree(){
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp) {
		BTComp->StopTree();
	}
}

void AWraithAIController::RestartBehaviourTree() {
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp) {
		BTComp->RestartTree();
	}
}