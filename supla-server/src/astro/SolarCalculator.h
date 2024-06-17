//======================================================================================================================
// SolarCalculator Library for Arduino
//
// This library provides functions to calculate the times of sunrise, sunset, solar noon, twilight (dawn and dusk),
// Sun's apparent position in the sky, equation of time, etc.
//
// Most formulae are taken from Astronomical Algorithms by Jean Meeus and optimized for Arduino.
//======================================================================================================================

// https://github.com/jpb10/SolarCalculator/blob/02e26c2caa6efb60de284046761fde422f43603f/LICENSE#L1

#ifndef SOLARCALCULATOR_H
#define SOLARCALCULATOR_H

//namespace solarcalculator {

constexpr double SUNRISESET_STD_ALTITUDE = -0.8333;
constexpr double CIVIL_DAWNDUSK_STD_ALTITUDE = -6.0;
constexpr double NAUTICAL_DAWNDUSK_STD_ALTITUDE = -12.0;
constexpr double ASTRONOMICAL_DAWNDUSK_STD_ALTITUDE = -18.0;

struct JulianDay
{
    double JD;  // Julian day at 0h UT (JD ending in .5)
    double m;   // Fractional day, 0h to 24h (decimal number between 0 and 1)

    explicit JulianDay(unsigned long utc);  // Unix time, i.e. seconds since 0h UT 1 January 1970
    JulianDay(int year, int month, int day, int hour = 0, int minute = 0, int second = 0);  // Calendar date (UTC)
};

//======================================================================================================================
// Intermediate calculations
//
// Time T is measured in Julian centuries (36525 ephemeris days from the epoch J2000.0)
//======================================================================================================================

// Utilities
double wrapTo360(double angle);
double wrapTo180(double angle);

// Julian centuries
double calcJulianCent(JulianDay jd);

// Solar coordinates
double calcGeomMeanLongSun(double T);
double calcGeomMeanAnomalySun(double T);
double calcSunEqOfCenter(double T);
double calcSunRadVector(double T);
double calcMeanObliquityOfEcliptic(double T);
void calcSolarCoordinates(double T, double &ra, double &dec);

// Sidereal time at Greenwich
double calcGrMeanSiderealTime(JulianDay jd);

// Sun's position in the sky
void equatorial2horizontal(double H, double dec, double lat, double &az, double &el);
double calcHourAngleRiseSet(double dec, double lat, double h0);
double calcRefraction(double el);

//======================================================================================================================
// Solar calculator
//
// All calculations assume time inputs in Coordinated Universal Time (UTC)
//======================================================================================================================

// Equation of time, in minutes of time
void calcEquationOfTime(JulianDay jd, double &E);

// Sun's geocentric (as seen from the center of the Earth) equatorial coordinates, in degrees and AUs
void calcEquatorialCoordinates(JulianDay jd, double &rt_ascension, double &declination, double &radius_vector);

// Sun's topocentric (as seen from the observer's place on the Earth's surface) horizontal coordinates, in degrees
void calcHorizontalCoordinates(JulianDay jd, double latitude, double longitude, double &azimuth, double &elevation);

// Find the times of sunrise, transit, and sunset, in hours
void calcSunriseSunset(JulianDay jd, double latitude, double longitude,
                       double &transit, double &sunrise, double &sunset,
                       double altitude = SUNRISESET_STD_ALTITUDE, int iterations = 1);

//======================================================================================================================
// Wrapper functions
//
// All calculations assume time inputs in Coordinated Universal Time (UTC)
//======================================================================================================================

void calcEquationOfTime(unsigned long utc, double &E);
void calcEquationOfTime(int year, int month, int day, int hour, int minute, int second, double &E);

void calcEquatorialCoordinates(unsigned long utc, double &rt_ascension, double &declination, double &radius_vector);
void calcEquatorialCoordinates(int year, int month, int day, int hour, int minute, int second,
                               double &rt_ascension, double &declination, double &radius_vector);

void calcHorizontalCoordinates(unsigned long utc, double latitude, double longitude,
                               double &azimuth, double &elevation);
void calcHorizontalCoordinates(int year, int month, int day, int hour, int minute, int second,
                               double latitude, double longitude, double &azimuth, double &elevation);

void calcSunriseSunset(unsigned long utc, double latitude, double longitude,
                       double &transit, double &sunrise, double &sunset,
                       double altitude = SUNRISESET_STD_ALTITUDE, int iterations = 1);
void calcSunriseSunset(int year, int month, int day, double latitude, double longitude,
                       double &transit, double &sunrise, double &sunset,
                       double altitude = SUNRISESET_STD_ALTITUDE, int iterations = 1);

void calcCivilDawnDusk(unsigned long utc, double latitude, double longitude,
                       double &transit, double &dawn, double &dusk);
void calcCivilDawnDusk(int year, int month, int day, double latitude, double longitude,
                       double &transit, double &dawn, double &dusk);

void calcNauticalDawnDusk(unsigned long utc, double latitude, double longitude,
                          double &transit, double &dawn, double &dusk);
void calcNauticalDawnDusk(int year, int month, int day, double latitude, double longitude,
                          double &transit, double &dawn, double &dusk);

void calcAstronomicalDawnDusk(unsigned long utc, double latitude, double longitude,
                              double &transit, double &dawn, double &dusk);
void calcAstronomicalDawnDusk(int year, int month, int day, double latitude, double longitude,
                              double &transit, double &dawn, double &dusk);

//}  // namespace
#endif  //SOLARCALCULATOR_H
