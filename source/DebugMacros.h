#pragma once

#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(Location) if (GetWorld()) { DrawDebugSphere(GetWorld(), Location, 25.f, 12, FColor::Red, true); }
#define DRAW_SPHERE_ONCE(Location) if (GetWorld()) { DrawDebugSphere(GetWorld(), Location, 25.f, 12, FColor::Red, false); }
#define DRAW_SPHERE_COLOR(Location, Color) if (GetWorld()) { DrawDebugSphere(GetWorld(), Location, 8.f, 12, Color, false, 5.f); }
#define DRAW_LINE(StartLoc, EndLoc) if (GetWorld()) { DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Blue, true); }
#define DRAW_LINE_ONCE(StartLoc, EndLoc) if (GetWorld()) { DrawDebugLine(GetWorld(), StartLoc, EndLoc, FColor::Blue, false); }
#define DRAW_POINT(Location) if (GetWorld()) { DrawDebugPoint(GetWorld(), Location, 25.f, FColor::Cyan, true); }
#define DRAW_POINT_ONCE(Location) if (GetWorld()) { DrawDebugPoint(GetWorld(), Location, 25.f, FColor::Cyan, false); }
#define DRAW_VECTOR(StartLoc, EndLoc) if (GetWorld()) \
	{ \
		DRAW_LINE(StartLoc, EndLoc) \
		DRAW_POINT(EndLoc) \
	} \

#define DRAW_VECTOR_ONCE(StartLoc, EndLoc) if (GetWorld()) \
	{ \
		DRAW_LINE_ONCE(StartLoc, EndLoc); \
		DRAW_POINT_ONCE(EndLoc); \
	} \
