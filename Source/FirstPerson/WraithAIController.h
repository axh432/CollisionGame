// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WraithAIController.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPERSON_API AWraithAIController : public AAIController
{
	GENERATED_BODY()
private: 
	ACharacter* FoundEnemy;

protected:
	virtual void BeginPlay();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		UBehaviorTree* BehaviourTree;

public:
	void StopBehaviourTree();

	void RestartBehaviourTree();

	void FindAnEnemy();
	
	void MoveCloseToEnemy();

	void AttackEnemy();
};
