#pragma once
#include "Expressions.h"
using namespace std;

//definimin for movie ganres
#define COMEDY 1
#define DRAMA 2
#define THRILLER 3
#define HORROR 4
#define ACTION 5

//vars for calculating precenteges
static long double smilePrecent;
static long double naturalPrecent;
static long double angryPrecent;
static long double surprisedPrecent;
static long double scaredPrecent;
static long double sadPrecent;

class MovieRatings
{
	public:
	MovieRatings();
	~MovieRatings();
	void calcStatistics(long double frameC, long double smileC, long double surprisedC, long double scaredC, long double sadC, long double angryC, long double naturalC);
};

