//======================================================================================================================
// SolarCalculator Library for Arduino
//
// This library provides functions to calculate the times of sunrise, sunset, solar noon, twilight (dawn and dusk),
// Sun's apparent position in the sky, equation of time, etc.
//
// Most formulae are taken from Astronomical Algorithms by Jean Meeus and optimized for Arduino.
//======================================================================================================================

// https://github.com/jpb10/SolarCalculator/blob/02e26c2caa6efb60de284046761fde422f43603f/LICENSE#L1

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cmath>
#endif

#include "SolarCalculator.h"

//namespace solarcalculator {

JulianDay::JulianDay(unsigned long utc)
{
    JD = static_cast<unsigned long>(utc / 86400) + 2440587.5;
    m = (utc % 86400) / 86400.0;
}

// Valid from 1901 to 2099, Van Flandern & Pulkkinen (1979)
JulianDay::JulianDay(int Y, int M, int D, int hour, int minute, int second)
{
    JD = 367.0 * Y - static_cast<int>(7 * (Y + (M + 9) / 12) / 4) + static_cast<int>(275 * M / 9) + D + 1721013.5;
    m = (hour + minute / 60.0 + second / 3600.0) / 24.0;
}

//======================================================================================================================
// Intermediate calculations
//
// Time T is measured in Julian centuries (36525 ephemeris days from the epoch J2000.0)
//======================================================================================================================

#ifndef ARDUINO
double radians(double deg)
{
    return deg * M_PI / 180;
}

double degrees(double rad)
{
    return rad * 180 / M_PI;
}
#endif

double wrapTo360(double angle)
{
    angle = fmod(angle, 360);
    if (angle < 0) angle += 360;
    return angle;  // [0, 360)
}

double wrapTo180(double angle)
{
    angle = wrapTo360(angle + 180);
    return angle - 180;  // [-180, 180)
}

double calcJulianCent(JulianDay jd)
{
    return (jd.JD - 2451545 + jd.m) / 36525;
}

double calcGeomMeanLongSun(double T)
{
    return wrapTo360(280.46646 + T * 36000.76983);  // in degrees
}

double calcGeomMeanAnomalySun(double T)
{
    return wrapTo360(357.52911 + T * 35999.05029);  // in degrees
}

double calcSunEqOfCenter(double T)
{
    double M = calcGeomMeanAnomalySun(T);
    return sin(radians(M)) * 1.914602 + sin(2 * radians(M)) * 0.019993;  // in degrees
}

double calcSunRadVector(double T)
{
    double M = calcGeomMeanAnomalySun(T);
    return 1.00014 - 0.01671 * cos(radians(M)) - 0.00014 * cos(2 * radians(M));  // in AUs
}

double calcMeanObliquityOfEcliptic(double T)
{
    return 23.4392911 - T * 0.0130042;  // in degrees
}

// Mean geocentric equatorial coordinates, accurate to ~0.01 degree
void calcSolarCoordinates(double T, double &ra, double &dec)
{
    double L0 = calcGeomMeanLongSun(T);
    double C = calcSunEqOfCenter(T);
    double L = L0 + C - 0.00569;  // corrected for aberration

    double eps = calcMeanObliquityOfEcliptic(T);
    ra = degrees(atan2(cos(radians(eps)) * sin(radians(L)), cos(radians(L))));  // [-180, 180)
    dec = degrees(asin(sin(radians(eps)) * sin(radians(L))));
}

double calcGrMeanSiderealTime(JulianDay jd)
{
    double GMST0 = wrapTo360(100.46061837 + 0.98564736629 * (jd.JD - 2451545));
    return wrapTo360(GMST0 + 360.985647 * jd.m);  // in degrees
}

void equatorial2horizontal(double H, double dec, double lat, double &az, double &el)
{
    az = degrees(atan2(sin(radians(H)), cos(radians(H)) * sin(radians(lat)) - tan(radians(dec)) * cos(radians(lat))));
    el = degrees(asin(sin(radians(lat)) * sin(radians(dec)) + cos(radians(lat)) * cos(radians(dec)) * cos(radians(H))));
}

// Hour angle at sunrise or sunset, returns NaN if circumpolar
double calcHourAngleRiseSet(double dec, double lat, double h0)
{
    return degrees(acos((sin(radians(h0)) - sin(radians(lat)) * sin(radians(dec))) /
                   (cos(radians(lat)) * cos(radians(dec)))));
}

// Approximate atmospheric refraction correction, in degrees
double calcRefraction(double el)
{
    if (el < -0.575)
        return -20.774 / tan(radians(el)) / 3600;  // Zimmerman (1981)
    else
        return 1.02 / tan(radians(el + 10.3 / (el + 5.11))) / 60;  // Sæmundsson (1986)
}

//======================================================================================================================
// Solar calculator
//
// All calculations assume time inputs in Coordinated Universal Time (UTC)
//======================================================================================================================

// Equation of time, in minutes of time
void calcEquationOfTime(JulianDay jd, double &E)
{
    double T = calcJulianCent(jd);
    double L0 = calcGeomMeanLongSun(T);

    double ra, dec;
    calcSolarCoordinates(T, ra, dec);

    E = 4 * wrapTo180(L0 - 0.00569 - ra);
}

// Sun's geocentric (as seen from the center of the Earth) equatorial coordinates, in degrees and AUs
void calcEquatorialCoordinates(JulianDay jd, double &rt_ascension, double &declination, double &radius_vector)
{
    double T = calcJulianCent(jd);
    calcSolarCoordinates(T, rt_ascension, declination);

    rt_ascension = wrapTo360(rt_ascension);
    radius_vector = calcSunRadVector(T);
}

// Sun's topocentric (as seen from the observer's place on the Earth's surface) horizontal coordinates, in degrees
void calcHorizontalCoordinates(JulianDay jd, double latitude, double longitude, double &azimuth, double &elevation)
{
    double T = calcJulianCent(jd);
    double GMST = calcGrMeanSiderealTime(jd);

    double ra, dec;
    calcSolarCoordinates(T, ra, dec);

    double H = GMST + longitude - ra;
    equatorial2horizontal(H, dec, latitude, azimuth, elevation);

    azimuth += 180;  // measured from the North
    elevation += calcRefraction(elevation);
}

// Find the times of sunrise, transit, and sunset, in hours
void calcSunriseSunset(JulianDay jd, double latitude, double longitude,
                       double &transit, double &sunrise, double &sunset, double altitude, int iterations)
{
    double m[3];
    m[0] = 0.5 - longitude / 360;

    for (int i = 0; i <= iterations; i++)
        for (int event = 0; event < 3; event++)
        {
            jd.m = m[event];
            double T = calcJulianCent(jd);
            double GMST = calcGrMeanSiderealTime(jd);

            double ra, dec;
            calcSolarCoordinates(T, ra, dec);

            double m0 = jd.m + wrapTo180(ra - longitude - GMST) / 360;
            double d0 = calcHourAngleRiseSet(dec, latitude, altitude) / 360;

            if (event == 0) m[0] = m0;
            if (event == 1 || i == 0) m[1] = m0 - d0;
            if (event == 2 || i == 0) m[2] = m0 + d0;
            if (i == 0) break;
        }

    transit = m[0] * 24;
    sunrise = m[1] * 24;
    sunset = m[2] * 24;
}

//======================================================================================================================
// Wrapper functions
//
// All calculations assume time inputs in Coordinated Universal Time (UTC)
//======================================================================================================================

void calcEquationOfTime(unsigned long utc, double &E)
{
    JulianDay jd(utc);
    calcEquationOfTime(jd, E);
}

void calcEquationOfTime(int year, int month, int day, int hour, int minute, int second, double &E)
{
    JulianDay jd(year, month, day, hour, minute, second);
    calcEquationOfTime(jd, E);
}

void calcEquatorialCoordinates(unsigned long utc, double &rt_ascension, double &declination, double &radius_vector)
{
    JulianDay jd(utc);
    calcEquatorialCoordinates(jd, rt_ascension, declination, radius_vector);
}

void calcEquatorialCoordinates(int year, int month, int day, int hour, int minute, int second,
                               double &rt_ascension, double &declination, double &radius_vector)
{
    JulianDay jd(year, month, day, hour, minute, second);
    calcEquatorialCoordinates(jd, rt_ascension, declination, radius_vector);
}

void calcHorizontalCoordinates(unsigned long utc, double latitude, double longitude,
                               double &azimuth, double &elevation)
{
    JulianDay jd(utc);
    calcHorizontalCoordinates(jd, latitude, longitude, azimuth, elevation);
}

void calcHorizontalCoordinates(int year, int month, int day, int hour, int minute, int second,
                               double latitude, double longitude, double &azimuth, double &elevation)
{
    JulianDay jd(year, month, day, hour, minute, second);
    calcHorizontalCoordinates(jd, latitude, longitude, azimuth, elevation);
}

void calcSunriseSunset(unsigned long utc, double latitude, double longitude,
                       double &transit, double &sunrise, double &sunset, double altitude, int iterations)
{
    JulianDay jd(utc);
    calcSunriseSunset(jd, latitude, longitude, transit, sunrise, sunset, altitude, iterations);
}

void calcSunriseSunset(int year, int month, int day, double latitude, double longitude,
                       double &transit, double &sunrise, double &sunset, double altitude, int iterations)
{
    JulianDay jd(year, month, day);
    calcSunriseSunset(jd, latitude, longitude, transit, sunrise, sunset, altitude, iterations);
}

void calcCivilDawnDusk(unsigned long utc, double latitude, double longitude,
                       double &transit, double &dawn, double &dusk)
{
    calcSunriseSunset(utc, latitude, longitude, transit, dawn, dusk, CIVIL_DAWNDUSK_STD_ALTITUDE);
}

void calcCivilDawnDusk(int year, int month, int day, double latitude, double longitude,
                       double &transit, double &dawn, double &dusk)
{
    calcSunriseSunset(year, month, day, latitude, longitude, transit, dawn, dusk, CIVIL_DAWNDUSK_STD_ALTITUDE);
}

void calcNauticalDawnDusk(unsigned long utc, double latitude, double longitude,
                          double &transit, double &dawn, double &dusk)
{
    calcSunriseSunset(utc, latitude, longitude, transit, dawn, dusk, NAUTICAL_DAWNDUSK_STD_ALTITUDE);
}

void calcNauticalDawnDusk(int year, int month, int day, double latitude, double longitude,
                          double &transit, double &dawn, double &dusk)
{
    calcSunriseSunset(year, month, day, latitude, longitude, transit, dawn, dusk, NAUTICAL_DAWNDUSK_STD_ALTITUDE);
}

void calcAstronomicalDawnDusk(unsigned long utc, double latitude, double longitude,
                              double &transit, double &dawn, double &dusk)
{
    calcSunriseSunset(utc, latitude, longitude, transit, dawn, dusk, ASTRONOMICAL_DAWNDUSK_STD_ALTITUDE);
}

void calcAstronomicalDawnDusk(int year, int month, int day, double latitude, double longitude,
                              double &transit, double &dawn, double &dusk)
{
    calcSunriseSunset(year, month, day, latitude, longitude, transit, dawn, dusk, ASTRONOMICAL_DAWNDUSK_STD_ALTITUDE);
}

//}  // namespace
