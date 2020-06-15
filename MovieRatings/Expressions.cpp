#include "Expressions.h"



Expressions::Expressions()
{
}

Expressions::~Expressions()
{
}

 void Expressions::expressionsRecogniser(sMarker *markersArray)
{
	expression.setFrameCount();
	 faceSetUp(markersArray);

	//measuring important distances
	float mouthWidth = markerDistance(markersArray[MOUTH_LEFT], markersArray[MOUTH_RIGHT]);
	float mouthHeight = markerDistance(markersArray[MOUTH_DOWN], markersArray[MOUTH_UP]);
	float eyeToBrowLeft = markerDistance(markersArray[EYE_LEFT], markersArray[BROW_LEFT]);
	float eyeToBrowRight = markerDistance(markersArray[EYE_RIGHT], markersArray[BROW_RIGHT]);
	float chinToLeftMouse = markerDistance(markersArray[MOUTH_DOWN], markersArray[MOUTH_LEFT]);
	float chinToRightMouse = markerDistance(markersArray[MOUTH_DOWN], markersArray[MOUTH_RIGHT]);
	float browsWidth = markerDistance(markersArray[BROW_LEFT], markersArray[BROW_RIGHT]);

	//first measuring flag to get the user natural face (OG distances)
	if (flag == 0)
	{
		//setting natural face distances
		mouthWidthOG = mouthWidth;
		mouthHeightOG = mouthHeight;
		eyeToBrowLeftOG = eyeToBrowLeft;
		eyeToBrowRightOG = eyeToBrowRight;
		chinToLeftMouseOG = chinToLeftMouse;
		chinToRightMouseOG = chinToRightMouse;
		browsWidthOG = browsWidth;
		flag++;
	}

	//expression flags
	bool smile = false;
	bool sad = false;
	bool angry = false;
	bool surprised = false;
	bool scared = false;

	//smile
	if (chinToLeftMouse > chinToLeftMouseOG*1.2 && chinToRightMouse > chinToRightMouseOG*1.2 && eyeToBrowLeft < eyeToBrowLeftOG*1.05 && eyeToBrowRight < eyeToBrowRightOG *1.05)
	{
		printf("SMILE :)))))\n");
		expression.setSmileCount();
		smile = true;
	}

	//surprised
	if (eyeToBrowLeft > eyeToBrowLeftOG*1.08 && eyeToBrowRight > eyeToBrowRightOG *1.08 && mouthHeight > mouthHeightOG*1.1 && !smile)
	{
		printf("SURPRISED :o \n");
		expression.setSurprisedCount();
		surprised = true;
	}

	//scared
	if (eyeToBrowLeft < eyeToBrowLeftOG*0.994 && eyeToBrowRight < eyeToBrowRightOG*0.994 && mouthHeight > mouthHeightOG*1.08 && !smile && !surprised)
	{
		printf("SCARED *O* \n");
		expression.setScardCount();
		scared = true;
	}

	//sad
	if (eyeToBrowLeft < eyeToBrowLeftOG*0.993 && eyeToBrowRight < eyeToBrowRightOG*0.993 && mouthWidth < mouthWidthOG*0.9 && !smile && !scared && !surprised)
	{
		printf("SAD :( \n");
		expression.setSadCount();
		sad = true;
	}

	//anger
	if (browsWidth < browsWidthOG*0.96 && !smile && !scared && !surprised && !sad)
	{
		printf("ANGRY ;O \n");
		expression.setAngryCount();
		angry = true;
	}

	//natural
	if (!smile && !angry && !surprised && !scared && !sad) {
		printf("NATUAL :| \n");
		expression.setNaturlCount();

	}
	return;
}

// return distance between 2 markers (A and B)
float Expressions::markerDistance(sMarker markerA, sMarker markerB)
{
	return sqrt(pow((markerA.x - markerB.x), 2) + pow((markerA.y - markerB.y), 2) + pow((markerA.z - markerB.z), 2));
}

//sort Marker by X or Y (if cordinate is false by Y, else by X)
void Expressions::sortMarkers(sMarker * obj, int n, bool cordinate)
{
	sMarker temp;
	int i, j;
	for (i = 0; i < n; i++)
	{
		for (j = n - 1; j > i; j--)
		{
			if ((cordinate == true && obj[j].x < obj[j - 1].x) || (cordinate == false && obj[j].y < obj[j - 1].y))
			{
				temp = obj[j];
				obj[j] = obj[j - 1];
				obj[j - 1] = temp;
			}
		}
	}
	return;
}

//sorting the markers array by our defined order
void Expressions::faceSetUp(sMarker * markersArray)
{
	//face Array markers algorythem
	//find mouth and putting it in 0-3
	sortMarkers(markersArray, NUMBER_OF_MARKRS, false);	//sort all the array by y
	sortMarkers(&markersArray[1], 2, true);	//sort moouth sides by x

	//find eyes and eyebrows and putting them in 4-7
	sortMarkers(&markersArray[4], 2, true);	//sort eyes by x
	sortMarkers(&markersArray[6], 2, true);	//sort eyebrows by x 
	return;
}

