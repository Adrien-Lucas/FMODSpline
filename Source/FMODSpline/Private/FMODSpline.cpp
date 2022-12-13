#include "FMODSpline.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AFMODSpline::AFMODSpline()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create the spline component
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));

    // Create the FMOD audio component
    FMODAudioComponent = CreateDefaultSubobject<UFMODAudioComponent>(TEXT("FMODAudioComponent"));
}

void AFMODSpline::BeginPlay()
{
    Super::BeginPlay();
    
    // Set the FMOD event on the audio component
    FMODAudioComponent->SetEvent(FMODEvent);

    // Play the FMOD event
    FMODAudioComponent->Play();
    
    // Set the value of the SplineBoundsRadius variable
    GetSplineBounds(SplineBoundsRadius, BoundsCenter);

    if (bShowDebug)
    {
    	DrawDebugCylinder(GetWorld(), BoundsCenter - FVector(0, 0, CylinderHeight/2.0f), BoundsCenter + FVector(0, 0, CylinderHeight/2.0f), SplineBoundsRadius, 32, FColor::Red, true, FLT_MAX, 0, 10);
    	// Draws the max distance
    	DrawDebugSphere(GetWorld(), BoundsCenter, MaxDistance, 32, FColor::Red, true, FLT_MAX, 0, 5);
    }
}

// Finds a random location that is contained within the spline
FVector AFMODSpline::GetRandomLocationInSplineShape(int iteration)
{
	iteration++;
	
    FVector RandomLocation;

    // Get a random point in the spline bounds
    RandomLocation.X = FMath::FRandRange(BoundsCenter.X - SplineBoundsRadius, BoundsCenter.X + SplineBoundsRadius);
    RandomLocation.Y = FMath::FRandRange(BoundsCenter.Y - SplineBoundsRadius, BoundsCenter.Y + SplineBoundsRadius);
    RandomLocation.Z = FMath::FRandRange(BoundsCenter.Z - CylinderHeight / 2.0f, BoundsCenter.Z + CylinderHeight / 2.0f);

	if(!IsPointInsideSplineShape(RandomLocation) || FVector::Dist(RandomLocation, ListenerTransform.GetLocation()) <= MinRandomSoundDistance)
    {
		// If too many iterations, stops searching
    	if(iteration > 25)
    		return RandomLocation;
		// Else tries again
    	return GetRandomLocationInSplineShape(iteration);
    }

	
    return RandomLocation;
}

void AFMODSpline::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	//Sets Listener location
	if(APlayerController* PC =GetWorld()->GetFirstPlayerController(); GetWorld() && IsValid(PC))
	{
		FVector ListenerPosition;
		FVector ListenerForward;
		FVector ListenerRight;
		PC->GetAudioListenerPosition(ListenerPosition, ListenerForward, ListenerRight);

		const FRotator Rotator = UKismetMathLibrary::MakeRotFromXY(ListenerForward, ListenerRight);

		ListenerTransform = FTransform(Rotator, ListenerPosition);
	}
	else
		return;
	
	
	//Disable the system if too far away from the player
	if(IsListenerTooFar())
		return;

    // Update the FMOD audio component's position
    UpdateFMODAudioComponentPosition(ListenerTransform.GetLocation());

	// Plays random sounds if the player is close enough and the sound is delayed already
    for(FRandomSound& RandomSound : RandomSounds)
	{
		if(!RandomSound.TimerHandle.IsValid())
		{
			// Start a timer that will call a lambda expression after a random delay
			GetWorld()->GetTimerManager().SetTimer(RandomSound.TimerHandle, [this, &RandomSound]()
			{

				if(RandomSound.EventInstance.Instance->isValid())
				{
					RandomSound.EventInstance.Instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
					RandomSound.EventInstance.Instance->release();
				}

				if(IsListenerTooFar())
					return;

				//Finds a random location contained within the spline shape
				const FVector RandomLocation = GetRandomLocationInSplineShape(0);
			
				// Play the FMOD event
				RandomSound.EventInstance = UFMODBlueprintStatics::PlayEventAtLocation(GetWorld(), RandomSound.Event, FTransform(RandomLocation), true);
				
				// Draw a debug sphere for 5 seconds at the random location
				if(bShowDebug)
					DrawDebugSphere(GetWorld(), RandomLocation, 100, 16, FColor::Green, false, 3, 0, 5);
				
				// Set the value of the TimerHandle variable to an invalid handle
				RandomSound.TimerHandle.Invalidate();


			}, FMath::RandRange(RandomSound.DelayRange.X, RandomSound.DelayRange.Y), false);
		}
	}
	
	if (bShowDebug)
	{
		const FVector AudioComponentPosition = FMODAudioComponent->GetComponentLocation();
		DrawDebugSphere(GetWorld(), AudioComponentPosition, 100.0f, 16, FColor::White, false, 0, 0, 2);
	}
}

// Returns if the player is too far
bool AFMODSpline::IsListenerTooFar() const
{
	return FVector::Dist(ListenerTransform.GetLocation(), BoundsCenter) > MaxDistance;
}

void AFMODSpline::UpdateFMODAudioComponentPosition(const FVector& ListenerPosition)
{
	// Check if the player is inside the shape defined by the spline points
	const bool bListenerInsideShape = IsPointInsideSplineShape(ListenerPosition);
	FVector NewLocation;
	
	// Get the closest point on the spline to the player's position
	const FVector ClosestPoint = Spline->FindLocationClosestToWorldLocation(ListenerPosition, ESplineCoordinateSpace::World);
	

	bool bInCylinder = false;
	// 2D distance between player position and the bounds center
	const float HorizontalDistance = FVector2D::Distance(FVector2D(ListenerPosition), FVector2D(BoundsCenter));
	const float VerticalDistance = abs(ListenerPosition.Z - BoundsCenter.Z);

	bInCylinder = HorizontalDistance < SplineBoundsRadius && VerticalDistance < CylinderHeight / 2.0f;
	
	// If the player is inside the shape and the spline is closed, set the FMOD audio component's position to the player's position
	if (bListenerInsideShape && Spline->IsClosedLoop() && bInCylinder)
	{
		NewLocation = ListenerTransform.GetLocation();
		FMODAudioComponent->SetWorldRotation(ListenerTransform.GetRotation());
		
		//If the player just entered the spline shape, plays the FMOD event
		if (!bListenerInsideSplineShape)
		{
			//Plays fmod reverb and mix event
			if(FMODReverbEvent)
				FMODReverbEventInstance = UFMODBlueprintStatics::PlayEvent2D(GetWorld(), FMODReverbEvent, true);
			if(FMODMixEvent)
				FMODMixEventInstance = UFMODBlueprintStatics::PlayEvent2D(GetWorld(), FMODMixEvent, true);
			bListenerInsideSplineShape = true;
		}
	}
	else
	{
		// If the player just exited the spline shape, stops the FMOD event
		if (bListenerInsideSplineShape)
		{
			//Stops fmod reverb and mix event
			FMODReverbEventInstance.Instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
			FMODMixEventInstance.Instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);

			//Release the fmod reverb and mix event
			FMODReverbEventInstance.Instance->release();
			FMODMixEventInstance.Instance->release();
			
			bListenerInsideSplineShape = false;
		}
	
		
		// Sets the closest point to the closest point on an opening
		if(Openings.Num() > 0)
		{
			float ClosestDist = FLT_MAX;
			NewLocation = FVector::Zero();
			
			FOpening ClosestOpening;
			for (FOpening Opening : Openings)
			{				
				FVector ClosestPointOnThisOpening = Opening.GetClosestPointOnOpening(ClosestPoint, Spline);
				float Dist = FVector::Distance(ClosestPointOnThisOpening, ClosestPoint);
				if(NewLocation == FVector::Zero() || Dist < ClosestDist)
				{
					NewLocation = ClosestPointOnThisOpening;
					ClosestDist = Dist;
					ClosestOpening = Opening;
				}
			}
			//Sets the bool param of the audio fmod component "Portal_IsOpen" to the value of the opening
			FMODAudioComponent->SetParameter("Portal_IsOpen", ClosestOpening.bOpen);
		}
		else
		{
			// Find the closest point on the spline to the player's position
			NewLocation = ClosestPoint;
		}
	}
	FMODAudioComponent->SetWorldLocation(NewLocation);
}

// Check if the given point is contained inside the shape defined by the points of the spline
bool AFMODSpline::IsPointInsideSplineShape(const FVector& Point) const
{
	// Get the points of the spline in world space
	TArray<FVector> SplinePoints;

	for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
	{
		SplinePoints.Add(Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
	}

	// Check if the point is inside the shape defined by the spline points
	const FVector2D Point2D(Point.X, Point.Y);
	int CrossingNumber = 0;
	for (int i = 0; i < SplinePoints.Num(); i++)
	{
		int j = (i + 1) % SplinePoints.Num();
		FVector2D A(SplinePoints[i].X, SplinePoints[i].Y);
		FVector2D B(SplinePoints[j].X, SplinePoints[j].Y);
		if (A.Y > B.Y)
		{
			Swap(A, B);
		}
		if (A.Y <= Point2D.Y && Point2D.Y < B.Y)
		{
			float t = (Point2D.Y - A.Y) / (B.Y - A.Y);
			if (Point2D.X < A.X + t * (B.X - A.X))
			{
				CrossingNumber++;
			}
		}
	}
	return CrossingNumber % 2 == 1;
}

void AFMODSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Flush all debug draw
	FlushPersistentDebugLines(GetWorld());

	//Editor only script
	if(!GIsEditor)
		return;
	
	if(!IsValid(Spline))
		return;

	// Set the value of the SplineBoundsRadius variable
	GetSplineBounds(SplineBoundsRadius, BoundsCenter);
	
	// Draws the max distance
	DrawDebugSphere(GetWorld(), BoundsCenter, MaxDistance, 32, FColor::Red, true, FLT_MAX, 0, 5);
	
	// Draws the openings
	for (int i = 0; i < Openings.Num(); i++)
	{
		FOpening Opening = Openings[i];

		const float OpeningsCenterAtDistance = Opening.GetCenterDistanceOnSpline(Spline);
		const float OpeningsStartAtDistance = Opening.GetStartDistanceOnSpline(Spline);
		const float OpeningsEndAtDistance = Opening.GetEndDistanceOnSpline(Spline);
		
		const int LineNum = Openings[i].Width / OpeningDebugPrecision;
		
		for(int j = 0; j < LineNum; j++)
		{
			FVector LineStart = Spline->GetLocationAtDistanceAlongSpline(OpeningsStartAtDistance + Opening.Width * j/LineNum, ESplineCoordinateSpace::World);

			FVector LineEnd = Spline->GetLocationAtDistanceAlongSpline(OpeningsEndAtDistance, ESplineCoordinateSpace::World);
			if(j+1 < LineNum)
				LineEnd = Spline->GetLocationAtDistanceAlongSpline(OpeningsStartAtDistance + Opening.Width * (j+1)/LineNum, ESplineCoordinateSpace::World);

			DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Yellow, true, FLT_MAX, 0, 20);
			DrawDebugPoint(GetWorld(), Spline->GetLocationAtDistanceAlongSpline(OpeningsCenterAtDistance, ESplineCoordinateSpace::World), 30, FColor::Red, true, FLT_MAX, 0);
		}
	}
}

//Finds the smallest possible sphere that can contain all spline points and return its radius and center
void AFMODSpline::GetSplineBounds(float& OutRadius, FVector& OutCenter) const
{
	OutRadius = 0;
	OutCenter = FVector::ZeroVector;
	
	if(!IsValid(Spline))
		return;
	
	TArray<FVector> SplinePoints;
	for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
	{
		SplinePoints.Add(Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
	}
	
	// Find the center of the points
	for (FVector Point : SplinePoints)
	{
		OutCenter += Point;
	}
	OutCenter /= SplinePoints.Num();
	
	// Find the radius of the sphere
	for (FVector Point : SplinePoints)
	{
		float Dist = FVector::Distance(Point, OutCenter);
		if(Dist > OutRadius)
			OutRadius = Dist;
	}
}

void AFMODSpline::SetOpeningState(int32 Index, bool bOpen)
{
	if(Index < 0 || Index >= Openings.Num())
		return;
	
	Openings[Index].bOpen = bOpen;
}