#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define SCAL_A 3.9083e-3
#define SCAL_B -5.775e-7
#define SCAL_C -4.183e-12

#define N_PWR       24
#define GAIN_VAL    1
#define RESIST_REF  1500

/* PT100 */
#define RESIST_ZERO_PT100       100
#define RESIST_MINUS_200_PT100  18.52
#define RESIST_PLUS_850_PT100   390.481
/* PT1000 */
#define RESIST_ZERO_PT1000      1000
#define RESIST_MINUS_200_PT1000 185.2
#define RESIST_PLUS_850_PT1000  3904.81

/* Default PT100 */
static int R_ZERO = RESIST_ZERO_PT100;
static int R_M200 = RESIST_MINUS_200_PT100;
static int R_P850 = RESIST_PLUS_850_PT100;

double rtd_resist_calculate(int code)
{
    double resist;
    int mid_point = 1 << (N_PWR - 1); // 2^(N-1)

    resist = ((double)(code - mid_point) * RESIST_REF) / (GAIN_VAL * mid_point);

    return resist;
}

bool r2t_transform(double r, double *t)
{
    bool ret;
    double temp, temp0;
    int index;

    if (t == NULL)
        return false;

    /* Get an initial temperature */
    temp0 = (r / R_ZERO - 1) / SCAL_A;
    
    /* Caculate approximation value */
    if (r >= R_M200 && r < R_ZERO) /* -200 ~ 0 degrees Celsius */
    {
        for (index = 0; index < 50; index++)
        {
            temp = temp0 + (r - R_ZERO * (1 + SCAL_A * temp0 + SCAL_B * temp0 * temp0 - 100 * SCAL_C * temp0 * temp0 * temp0 + SCAL_C * temp0 * temp0 * temp0 * temp0)) /
                                (R_ZERO * (SCAL_A + 2 * SCAL_B * temp0 - 300 * SCAL_C * temp0 * temp0 + 4 * SCAL_C * temp0 * temp0 * temp0));
            
            if (fabs(temp - temp0) < 0.001) {
                break;
            } else {
                temp0 = temp;
            }
        }
        *t = temp;
        ret = true;
    }
    else if (r >= R_ZERO && r <= R_P850) /* 0 ~ 850 degrees Celsius */
    {
        for (index = 0; index < 50; index++)
        {
            temp = temp0 + (r - R_ZERO * (1 + SCAL_A * temp0 + SCAL_B * temp0 * temp0)) / (R_ZERO * (SCAL_A + 2 * SCAL_B * temp0));

            if (fabs(temp - temp0) < 0.001) {
                break;
            } else {
                temp0 = temp;
            }
        }
        *t = temp;
        ret = true;
    }
    else
    {
        ret = false;
    }
    
    return ret;
}

void print_help(const char *prog_name)
{
    printf("Usage: %s --sensor [0|1] --raw [voltage_raw]\n", prog_name);
    printf("Options:\n");
    printf("  --sensor [0|1]    Sensor type: 0 for PT100, 1 for PT1000\n");
    printf("  --raw [value]    Voltage raw value\n");
    printf("  --help            Display this help message\n");
}

int main(int argc, char const *argv[])
{
    double r, t;
    bool result;
    int sensor_type = 0;  // default sensor PT100
    int voltage_raw = 0;

    if (argc < 5) {
        print_help(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--sensor") == 0) {
            if (i + 1 < argc) {
                sensor_type = atoi(argv[++i]);
            } else {
                print_help(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--raw") == 0) {
            if (i + 1 < argc) {
                voltage_raw = atoi(argv[++i]);
            } else {
                print_help(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else {
            print_help(argv[0]);
            return 1;
        }
    }

    if (sensor_type == 0)
    {
        R_ZERO = RESIST_ZERO_PT100;
        R_M200 = RESIST_MINUS_200_PT100;
        R_P850 = RESIST_PLUS_850_PT100;
    }
    else if (sensor_type == 1)
    {
        R_ZERO = RESIST_ZERO_PT1000;
        R_M200 = RESIST_MINUS_200_PT1000;
        R_P850 = RESIST_PLUS_850_PT1000;
    }
    else
    {
        print_help(argv[0]);
        return 1;
    }

    r = rtd_resist_calculate(voltage_raw);  // Get Resist RTD
    r *= 2; // Temporarily (need confirm)
    result = r2t_transform(r, &t);

    if (result) {
        printf("Temperature = %.2f degrees Celsius\n", t);
    } else {
        printf("The resistance value is out of range.\n");
    }

    return 0;
}

#if 0
int main(int argc, char const *arg[])
{
    double r, t;
    bool result;
    int sensor_type = 0;  // default sensor PT100
    int voltage_raw = 0;

    printf("Please enter the sensor type (0: PT100, 1 for PT1000): ");
    scanf("%d", &sensor_type);

    //printf("Please enter the resistance value (in ohms): ");
    //scanf("%lf", &r);

    if (sensor_type == 0)
    {
        R_ZERO = RESIST_ZERO_PT100;
        R_M200 = RESIST_MINUS_200_PT100;
        R_P850 = RESIST_PLUS_850_PT100;
    }
    else if (sensor_type == 1)
    {
        R_ZERO = RESIST_ZERO_PT1000;
        R_M200 = RESIST_MINUS_200_PT1000;
        R_P850 = RESIST_PLUS_850_PT1000;
    }

    printf("Please enter the voltage raw value : ");
    scanf("%d", &voltage_raw);

    r = rtd_resist_calculate(voltage_raw);  // Get Resist RTD
    result = r2t_transform(r, &t);

    if (result) {
        printf("Temperature = %.2f degrees Celsius\n", t);
    } else {
        printf("The resistance value is out of range.\n");
    }

    return 0;
}
#endif