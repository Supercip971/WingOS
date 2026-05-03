#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include <float.h>


#ifdef __LSP
#ifndef __WTHROW
#    ifdef __cplusplus
#        define __WTHROW noexcept
#    else
#        define __WTHROW
#    endif
#endif
#elifndef __WTHROW
#define __WTHROW
#else
#define __WTHROW
#endif
#ifndef NAN
#    define NAN (0.0 / 0.0)
#endif

#ifndef INFINITY
#    define INFINITY (1.0 / 0.0)
#endif
#define PI (3.14159265358979323846264338327f)
#define M_PI PI
#define M_PI_2 (M_PI / 2.0)
#define M_PI_4 (M_PI / 4.0)

#define M_1_PI 0.31830988618379067154 /* 1/pi */
#define M_2_PI (M_PI * 2.0)
#define M_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */

#if defined(_XHJ_OPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_SKIFT_SOURCE)
#    define M_E 2.7182818284590452354       /* e */
#    define M_LOG2E 1.4426950408889634074   /* log_2 e */
#    define M_LOG10E 0.43429448190325182765 /* log_10 e */
#    define M_LN2 0.69314718055994530942    /* log_e 2 */
#    define M_LN10 2.30258509299404568402   /* log_e 10 */

#    define M_SQRT2 1.41421356237309504880   /* sqrt(2) */
#    define M_SQRT1_2 0.70710678118654752440 /* 1/sqrt(2) */

extern int signgam;

double j0(double) __WTHROW;
double j1(double) __WTHROW;
double jn(int, double) __WTHROW;

double y0(double) __WTHROW;
double y1(double) __WTHROW;
double yn(int, double) __WTHROW;

#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_SKIFT_SOURCE)
#    define HUGE 3.40282346638528859812e+38F

double drem(double, double) __WTHROW;
float dremf(float, float) __WTHROW;

int finite(double) __WTHROW;
int finitef(float) __WTHROW;

double scalb(double, double) __WTHROW;
float scalbf(float, float) __WTHROW;

double significand(double) __WTHROW;
float significandf(float) __WTHROW;

double lgamma_r(double, int *) __WTHROW;
float lgammaf_r(float, int *) __WTHROW;

float j0f(float) __WTHROW;
float j1f(float) __WTHROW;
float jnf(int, float) __WTHROW;

float y0f(float) __WTHROW;
float y1f(float) __WTHROW;
float ynf(int, float) __WTHROW;

#endif

typedef double double_t;
typedef float float_t;

#define HUGE_VAL (__builtin_huge_val())
#define FP_ILOGBNAN (-1 - (int)(((unsigned)-1) >> 1))
#define FP_ILOGB0 FP_ILOGBNAN

#define signbit(x) (__builtin_signbit(x))

// [C11/7.12.3 Classification macros]

// NOTE: fpclassify always returns exactly one of those constants
// However making them bitwise disjoint simplifies isfinite() etc.
#define FP_INFINITE 1
#define FP_NAN 2
#define FP_NORMAL 4
#define FP_SUBNORMAL 8
#define FP_ZERO 16

int __fpclassify(double x) __WTHROW;
int __fpclassifyf(float x) __WTHROW;
int __fpclassifyl(long double x) __WTHROW;

#define fpclassify(x) \
    (sizeof(x) == sizeof(double) ? __fpclassify(x) : (sizeof(x) == sizeof(float) ? __fpclassifyf(x) : (sizeof(x) == sizeof(long double) ? __fpclassifyl(x) : 0)))

#define isfinite(x) (fpclassify(x) & (FP_NORMAL | FP_SUBNORMAL | FP_ZERO))
#define isnan(x) (fpclassify(x) == FP_NAN)
#define isinf(x) (fpclassify(x) == FP_INFINITE)
#define isnormal(x) (fpclassify(x) == FP_NORMAL)

// [ GNUC extension]

void sincos(double, double *, double *) __WTHROW;
void sincosf(float, float *, float *) __WTHROW;
void sincosl(long double, long double *, long double *) __WTHROW;

double exp10(double) __WTHROW;
float exp10f(float) __WTHROW;
long double exp10l(long double) __WTHROW;

double pow10(double) __WTHROW;
float pow10f(float) __WTHROW;
long double pow10l(long double) __WTHROW;

// [C11/7.12.4 Trigonometric functions]

double acos(double x) __WTHROW;
float acosf(float x) __WTHROW;
long double acosl(long double x) __WTHROW;
double asin(double x) __WTHROW;
float asinf(float x) __WTHROW;
long double asinl(long double x) __WTHROW;
double atan(double x) __WTHROW;
float atanf(float x) __WTHROW;
long double atanl(long double x) __WTHROW;
double atan2(double x, double y) __WTHROW;
float atan2f(float x, float y) __WTHROW;
long double atan2l(long double x, long double y) __WTHROW;
double cos(double x) __WTHROW;
float cosf(float x) __WTHROW;
long double cosl(long double x) __WTHROW;
double sin(double x) __WTHROW;
float sinf(float x) __WTHROW;
long double sinl(long double x) __WTHROW;
double tan(double x) __WTHROW;
float tanf(float x) __WTHROW;
long double tanl(long double x) __WTHROW;

// [C11/7.12.5 Hyperbolic functions]

double acosh(double x) __WTHROW;
float acoshf(float x) __WTHROW;
long double acoshl(long double x) __WTHROW;

double asinh(double x) __WTHROW;
float asinhf(float x) __WTHROW;
long double asinhl(long double x) __WTHROW;

double atanh(double x) __WTHROW;
float atanhf(float x) __WTHROW;
long double atanhl(long double x) __WTHROW;

double cosh(double x) __WTHROW;
float coshf(float x) __WTHROW;
long double coshl(long double x) __WTHROW;

double sinh(double x) __WTHROW;
float sinhf(float x) __WTHROW;
long double sinhl(long double x) __WTHROW;

double tanh(double x) __WTHROW;
float tanhf(float x) __WTHROW;
long double tanhl(long double x) __WTHROW;

// [C11/7.12.6 Exponential and logarithmic functions]

double exp(double x) __WTHROW;
float expf(float x) __WTHROW;
long double expl(long double x) __WTHROW;

double exp2(double x) __WTHROW;
float exp2f(float x) __WTHROW;
long double exp2l(long double x) __WTHROW;

double expm1(double x) __WTHROW;
float expm1f(float x) __WTHROW;
long double expm1l(long double x) __WTHROW;

double frexp(double x, int *power) __WTHROW;
float frexpf(float x, int *power) __WTHROW;
long double frexpl(long double x, int *power) __WTHROW;

int ilogb(double x) __WTHROW;
int ilogbf(float x) __WTHROW;
int ilogbl(long double x) __WTHROW;

double ldexp(double x, int power) __WTHROW;
float ldexpf(float x, int power) __WTHROW;
long double ldexpl(long double x, int power) __WTHROW;

#ifdef DO_NOT_LOG
double log(double x) __WTHROW;
#endif
float logf(float x) __WTHROW;
long double logl(long double x) __WTHROW;

double log10(double x) __WTHROW;
float log10f(float x) __WTHROW;
long double log10l(long double x) __WTHROW;

double log1p(double x) __WTHROW;
float log1pf(float x) __WTHROW;
long double log1pl(long double x) __WTHROW;

double log2(double x) __WTHROW;
float log2f(float x) __WTHROW;
long double log2l(long double x) __WTHROW;

double logb(double x) __WTHROW;
float logbf(float x) __WTHROW;
long double logbl(long double x) __WTHROW;

double modf(double x, double *integral) __WTHROW;
float modff(float x, float *integral) __WTHROW;
long double modfl(long double x, long double *integral) __WTHROW;

double scalbn(double x, int power) __WTHROW;
float scalbnf(float x, int power) __WTHROW;
long double scalbnl(long double x, int power) __WTHROW;

double scalbln(double x, long power) __WTHROW;
float scalblnf(float x, long power) __WTHROW;
long double scalblnl(long double x, long power) __WTHROW;

// [C11/7.12.7 Power and absolute-value functions]

double cbrt(double x) __WTHROW;
float cbrtf(float x) __WTHROW;
long double cbrtl(long double x) __WTHROW;

double fabs(double x) __WTHROW;
float fabsf(float x) __WTHROW;
long double fabsl(long double x) __WTHROW;

double hypot(double x, double y) __WTHROW;
float hypotf(float x, float y) __WTHROW;
long double hypotl(long double x, long double y) __WTHROW;

double pow(double x, double y) __WTHROW;
float powf(float x, float y) __WTHROW;
long double powl(long double x, long double y) __WTHROW;

double sqrt(double x) __WTHROW;
float sqrtf(float x) __WTHROW;
long double sqrtl(long double x) __WTHROW;

// [C11/7.12.8 Error and gamma functions]

double erf(double x) __WTHROW;
float erff(float x) __WTHROW;
long double erfl(long double x) __WTHROW;

double erfc(double x) __WTHROW;
float erfcf(float x) __WTHROW;
long double erfcl(long double x) __WTHROW;

double lgamma(double x) __WTHROW;
float lgammaf(float x) __WTHROW;
long double lgammal(long double x) __WTHROW;

double tgamma(double x) __WTHROW;
float tgammaf(float x) __WTHROW;
long double tgammal(long double x) __WTHROW;

// [C11/7.12.9 Nearest integer functions]

double ceil(double x) __WTHROW;
float ceilf(float x) __WTHROW;
long double ceill(long double x) __WTHROW;

double floor(double x) __WTHROW;
float floorf(float x) __WTHROW;
long double floorl(long double x) __WTHROW;

double nearbyint(double x) __WTHROW;
float nearbyintf(float x) __WTHROW;
long double nearbyintl(long double x) __WTHROW;

double rint(double x) __WTHROW;
float rintf(float x) __WTHROW;
long double rintl(long double x) __WTHROW;

long lrint(double x) __WTHROW;
long lrintf(float x) __WTHROW;
long lrintl(long double x) __WTHROW;

long long llrint(double x) __WTHROW;
long long llrintf(float x) __WTHROW;
long long llrintl(long double x) __WTHROW;

double round(double x) __WTHROW;
float roundf(float x) __WTHROW;
long double roundl(long double x) __WTHROW;

long lround(double x) __WTHROW;
long lroundf(float x) __WTHROW;
long lroundl(long double x) __WTHROW;

long long llround(double x) __WTHROW;
long long llroundf(float x) __WTHROW;
long long llroundl(long double x) __WTHROW;

double trunc(double x) __WTHROW;
float truncf(float x) __WTHROW;
long double truncl(long double x) __WTHROW;

// [C11/7.12.10 Remainder functions]

double fmod(double x, double y) __WTHROW;
float fmodf(float x, float y) __WTHROW;
long double fmodl(long double x, long double y) __WTHROW;

double remainder(double x, double y) __WTHROW;
float remainderf(float x, float y) __WTHROW;
long double remainderl(long double x, long double y) __WTHROW;

double remquo(double x, double y, int *quotient) __WTHROW;
float remquof(float x, float y, int *quotient) __WTHROW;
long double remquol(long double x, long double y, int *quotient) __WTHROW;

// [C11/7.12.11 Manipulation functions]

double copysign(double x, double sign) __WTHROW;
float copysignf(float x, float sign) __WTHROW;
long double copysignl(long double x, long double sign) __WTHROW;

double nan(char const *tag) __WTHROW;
float nanf(char const *tag) __WTHROW;
long double nanl(char const *tag) __WTHROW;

double nextafter(double x, double dir) __WTHROW;
float nextafterf(float x, float dir) __WTHROW;
long double nextafterl(long double x, long double dir) __WTHROW;

double nexttoward(double x, long double dir) __WTHROW;
float nexttowardf(float x, long double dir) __WTHROW;
long double nexttowardl(long double x, long double dir) __WTHROW;

// [C11/7.12.12 Maximum, minimum and positive difference functions]

double fdim(double x, double y) __WTHROW;
float fdimf(float x, float y) __WTHROW;
long double fdiml(long double x, long double y) __WTHROW;

double fmax(double x, double y) __WTHROW;
float fmaxf(float x, float y) __WTHROW;
long double fmaxl(long double x, long double y) __WTHROW;

double fmin(double x, double y) __WTHROW;
float fminf(float x, float y) __WTHROW;
long double fminl(long double x, long double y) __WTHROW;

// [C11/7.12.13 Floating multiply-add]

double fma(double, double, double) __WTHROW;
float fmaf(float, float, float) __WTHROW;
long double fmal(long double, long double, long double) __WTHROW;

#ifdef __cplusplus
}
#endif
