#pragma once
#include <iostream>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>

using namespace std;

//definimim for face array
#define NUMBER_OF_MARKRS 8
#define MOUTH_DOWN 0
#define MOUTH_RIGHT 1
#define MOUTH_LEFT 2
#define MOUTH_UP 3
#define EYE_RIGHT 4
#define EYE_LEFT 5
#define BROW_RIGHT 6
#define BROW_LEFT 7

//global vars
static int flag = 0;
static float mouthWidthOG;
static float mouthHeightOG;
static float chinToLeftMouseOG;
static float chinToRightMouseOG;
static float eyeToBrowLeftOG;
static float eyeToBrowRightOG;
static float browsWidthOG;

class Expressions
{
private:
	long double framesCount;
	long double naturalCount;
	long double smileCount;
	long double angryCount;
	long double surprisedCount;
	long double scaredCount;
	long double sadCount;

public:
	Expressions();
	~Expressions();
	static void expressionsRecogniser(sMarker* markersArray);
	static float markerDistance(sMarker markerA, sMarker markerB);
	static void sortMarkers(sMarker *obj, int n, bool cordinate);
	static void faceSetUp(sMarker* markersArray);
	//seters
	void setFrameCount() { framesCount++;}
	void setNaturlCount() { naturalCount++;}
	void setSmileCount() { smileCount++;}
	void setAngryCount() { angryCount++;}
	void setSurprisedCount() { surprisedCount++;}
	void setScardCount() { scaredCount++;}
	void setSadCount() { sadCount++;}
	//geters
	long double getFramesCount() { return framesCount; }
	long double getNaturalCount() { return naturalCount; }
	long double getSmileCount() { return  smileCount; }
	long double getAngryCount() { return angryCount; }
	long double getSurprisedCount() { return surprisedCount; }
	long double getScaredCount() { return scaredCount; }
	long double getSadCount() { return sadCount; }
};

//external instace per run time
extern Expressions expression;