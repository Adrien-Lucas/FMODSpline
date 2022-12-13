#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FMODBlueprintStatics.h"
#include "Components/SplineComponent.h"

#include "FMODSpline.generated.h"

// A struct that defines an opening
USTRUCT(BlueprintType)
struct FOpening
{
	GENERATED_BODY()

	// The position of the opening
	UPROPERTY(EditAnywhere, Category = Opening, meta=(MakeEditWidget=true))
	int SplinePoint;
	
	// The width of the opening
	UPROPERTY(EditAnywhere, Category = Opening)
	float Width = 100;

	// The state of the opening
	UPROPERTY(EditAnywhere, Category = Opening)
	bool bOpen = true;

	float GetCenterDistanceOnSpline(USplineComponent* Spline) const
	{
		return Spline->GetDistanceAlongSplineAtSplinePoint(SplinePoint);
	}

	float GetStartDistanceOnSpline(USplineComponent* Spline) const
	{
		float StartAtDistance = GetCenterDistanceOnSpline(Spline) - Width/2;
		if(StartAtDistance < 0)
			StartAtDistance += Spline->GetSplineLength();

		return StartAtDistance;
	}

	float GetEndDistanceOnSpline(USplineComponent* Spline) const
	{
		return GetCenterDistanceOnSpline(Spline) + Width/2;
	}

	FVector GetClosestPointOnOpening(FVector Point, USplineComponent* Spline) const
	{
		const float LocationToKey = Spline->FindInputKeyClosestToWorldLocation(Point);
		const float PointDistanceOnSpline = Spline->GetDistanceAlongSplineAtSplineInputKey(LocationToKey);

		if( PointDistanceOnSpline >= GetStartDistanceOnSpline(Spline) && PointDistanceOnSpline <= GetEndDistanceOnSpline(Spline))
			return Point;
		
		if(PointDistanceOnSpline < GetStartDistanceOnSpline(Spline))
			return Spline->GetLocationAtDistanceAlongSpline(GetStartDistanceOnSpline(Spline), ESplineCoordinateSpace::World);

		if(PointDistanceOnSpline > GetEndDistanceOnSpline(Spline))
			return Spline->GetLocationAtDistanceAlongSpline(GetEndDistanceOnSpline(Spline), ESplineCoordinateSpace::World);

		return FVector::Zero();
	}
};

USTRUCT()
struct FRandomSound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UFMODEvent* Event;

	// Range of the delay of the random sound
	UPROPERTY(EditAnywhere)
	FVector2D DelayRange;
	
	//Timer handle
	FTimerHandle TimerHandle;

	// Event instance
	FFMODEventInstance EventInstance;
	
};

UCLASS()
class FMODSPLINE_API AFMODSpline : public AActor
{
	// Rename the class from AMySplineActor to AFMODSpline
	GENERATED_BODY()
	
public:
	// The spline component for this actor
	UPROPERTY(VisibleAnywhere, Category = "Config")
	USplineComponent* Spline;

	// The FMOD event to play
	UPROPERTY(EditAnywhere, Category = "Config")
	UFMODEvent* FMODEvent;

	// The Reverb FMOD event to play when player enters the volume
	UPROPERTY(EditAnywhere, Category = "Config")
	UFMODEvent* FMODReverbEvent;

	// The Mix FMOD event to play when player enters the volume
	UPROPERTY(EditAnywhere, Category = "Config")
	UFMODEvent* FMODMixEvent;

	// The height of the cylinder
	UPROPERTY(EditAnywhere, Category = "Config")
	float CylinderHeight = 500.0f;

	// The max distance of the system
	UPROPERTY(EditAnywhere, Category = "Config")
	float MaxDistance = 2000.0f;

	// An array of UOpeningComponent objects for the openings on the spline
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FOpening> Openings;

	// Array of random sounds to play
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FRandomSound> RandomSounds;

	// The minimal distance a random sound can be played from the player
	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "RandomSounds.Num() > 0"))
	float MinRandomSoundDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Debug")
	bool bShowDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Debug")
	float OpeningDebugPrecision = 10;

	// Sets the state of an opening by its index
	UFUNCTION(BlueprintCallable)
	void SetOpeningState(int32 Index, bool bOpen);

	// The FMOD event instance
	UPROPERTY(BlueprintReadOnly)
	UFMODAudioComponent* FMODAudioComponent;
	
private:
	UPROPERTY()
	float SplineBoundsRadius = 0.0f;

	UPROPERTY()
	FVector BoundsCenter;
	
	UPROPERTY()
	FTransform ListenerTransform;
	
	UPROPERTY()
	bool bListenerInsideSplineShape;

	// The instance of FMODMixEvent
	UPROPERTY()
	FFMODEventInstance FMODMixEventInstance;

	// The instance of FMODReverEvent
	UPROPERTY()
	FFMODEventInstance FMODReverbEventInstance;

protected:
	// Sets default values for this actor's properties
	AFMODSpline();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FVector GetRandomLocationInSplineShape(int iteration);
	bool IsListenerTooFar() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void UpdateFMODAudioComponentPosition(const FVector& ListenerPosition);
	bool IsPointInsideSplineShape(const FVector& Point) const;
	virtual void OnConstruction(const FTransform& Transform) override;
	void GetSplineBounds(float& OutRadius, FVector& OutCenter) const;
};
