#include "Ratings.h"

MovieRatings::MovieRatings()
{
}

MovieRatings::~MovieRatings()
{
}

//calculating and presenting statistics based on expression precenteges
void MovieRatings::calcStatistics(long double frameC, long double smileC, long double surprisedC, long double scaredC, long double sadC, long double angryC, long double naturalC)
{
	int movieType = 0;

	//calculating statistics - deviding each expression frames by overall frames times 100
	naturalPrecent = naturalC / frameC * 100;
	smilePrecent = smileC / frameC * 100;
	angryPrecent = angryC / frameC * 100;
	surprisedPrecent = surprisedC / frameC * 100;
	scaredPrecent = scaredC / frameC * 100;
	sadPrecent = sadC / frameC * 100;

	cout << endl << "Choose movie type:\n 1: Comedy \n 2: Drama \n 3: Thriller \n 4: Horror \n 5: Action \n";
	cin >> movieType;
	getchar();

	//calulating movie ratings based on the ganre
	switch(movieType)
	{
	case COMEDY:
		if (smilePrecent > 50 && surprisedPrecent>5) {
			printf("The movie was funny, the audience were laugthing ");
		}
		else {
			printf("The movie was Not funny, the audience were laugthing only ");
		}
		printf("%.2f%% of the movie.", smilePrecent);
		break;

	case DRAMA:
		if (sadPrecent > 30 && angryPrecent > 5) {
			printf("The movie was heartbreaking!, the audience were sad ");
		}
		else {
			printf("The movie was Not emotional, the audience were sad only ");
		}
		printf("%.2f%% of the movie.", sadPrecent);
		break;

	case THRILLER:
		if (surprisedPrecent > 10 && angryPrecent > 5) {
			printf("The movie was thrilling, the audience were surprised ");
		}
		else {
			printf("The movie was  boring, the audience were surprised only ");
		}
		printf("%.2f%% of the movie.", surprisedPrecent);
		break;

	case HORROR:
		if (scaredPrecent > 25 && surprisedPrecent > 10) {
			printf("The movie was so scary!, the audience were scard ");
		}
		else {
			printf("The movie is suitable for small childrens, the audience were scard only ");
		}
		printf("%.2f%% of the movie.", scaredPrecent);
		break;

	case ACTION:
		if (scaredPrecent > 10 && surprisedPrecent > 10) {
			printf("The movie was AMAZING!, the audience were scared and surprised more then ");
		}
		else {
			printf("The movie was boring, the audience were scared and surprised only ");
		}
		printf("%.2f%% of the movie.", smilePrecent);
		break;

	default:
		printf("Error!");
		}

	//printing statistics
	printf("\n\nYour facial expressions statistics were:\n");
	printf("Natural Precent = %.2f%%\n", naturalPrecent);
	printf("Smile precent = %.2f%%\n", smilePrecent);
	printf("Angry precent = %.2f%%\n", angryPrecent);
	printf("Surprised precent = %.2f%%\n", surprisedPrecent);
	printf("Scared precent = %.2f%%\n", scaredPrecent);
	printf("Sad precent = %.2f%%\n", sadPrecent);
	printf("\n\nThanks for using Shay`s an Galit`s aplication :)\n");
	getchar();
	return;
}
