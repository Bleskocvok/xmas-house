
#include <stdio.h>
#include <stdlib.h>

#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include <PicoTM1637.h>


#define SIZE(arr) ( sizeof( arr ) / sizeof( *arr ) )
#define PWM_MAX 31


typedef struct
{
    char str[ 5 ];
    uint duration;
    uint data;

} dig_step_t;


typedef struct
{
    uint pin,
         pwm,
         chn;
} led_t;


#define TOP_STRIP 0
#define TOP_SIZE  8
#define RGB_R     8
#define RGB_G     9
#define RGB_B    10
#define BUILTIN  11


led_t strip[] =
{
    {  2 }, {  3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 }, { 9 },   // top strip
    { 14 }, { 10 }, { 11 },                                     // rgb led
    { 25 },                                                     // builtin led
};


led_t rgb[3] =
{
    { 18 }, { 20 }, { 19 },
};


void led_init( led_t* led )
{
    gpio_set_function( led->pin, GPIO_FUNC_PWM );

    led->pwm = pwm_gpio_to_slice_num( led->pin );
    led->chn = pwm_gpio_to_channel( led->pin );

    pwm_set_wrap( led->pwm, PWM_MAX );
    pwm_set_chan_level( led->pwm, led->chn, 0 );
    pwm_set_enabled( led->pwm, true );
}


void led_set_brightness( led_t* led, uint val )
{
    pwm_set_chan_level( led->pwm, led->chn, val );
}


typedef struct
{
    uint t;
    uint mod;
    uint dig;

} alternate_state_t;


void alternate( alternate_state_t* state )
{
    static const dig_step_t dig_effect[] =
    {
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
        { "Stas", 3 },
        { "-tne", 3 },
        { "a   ", 3 },
        { "Uese", 3 },
        { "-le ", 3 },
        { "UaNo", 3 },
        { "-Ce ", 3 },
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
        { "o0o0" }, { "0o0o" },
    };

    dig_step_t dig = dig_effect[ state->dig ];
    if ( dig.data == 0 )
        TM1637_display_word( dig.str, true );
    else
        TM1637_put_4_bytes( 0, dig.data );

    uint t = state->t % ( PWM_MAX * 2 + 1 );
    uint b = t <= PWM_MAX
           ? t
           : PWM_MAX - ( t - PWM_MAX );

    for (int i = 0; i < TOP_SIZE; i++)
    {
        if ( i % 2 == state->mod )
        {
            led_set_brightness( &strip[ TOP_STRIP + i ], b );
        }
    }

    if ( state->mod == 0 )
    {
        // yellow
        led_set_brightness( &strip[ RGB_R ], b / 2 );
        led_set_brightness( &strip[ RGB_G ], b / 2 );
    }
    else
    {
        // cyan
        led_set_brightness( &strip[ RGB_R ], b / 2 );
        led_set_brightness( &strip[ RGB_B ], b / 2 );
    }

    if ( state->t % ( PWM_MAX * 2 + 1 ) == ( PWM_MAX * 2 ) )
    {
        state->mod = 1 - state->mod;
        state->dig = ( state->dig + 1 ) % SIZE( dig_effect );
    }
    state->t++;
}


typedef struct
{
    uint t;
    int idx;

} crawling_state_t;


void crawling( crawling_state_t* state )
{
    static const uint8_t dig_eff[] =
    {
        0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000,
        0b00000001,
    };

    static const int dig_period = 4;

    uint dt = ( state->t / dig_period ) % sizeof( dig_eff );
    uint di = ( state->t / dig_period ) / sizeof( dig_eff ) % 4;
    TM1637_put_4_bytes( di, dig_eff[ dt ] );

    uint t = state->t % ( PWM_MAX * 2 + 1 );
    uint b = t <= PWM_MAX
           ? t
           : PWM_MAX - ( t - PWM_MAX );

    led_set_brightness( &strip[ TOP_STRIP + state->idx ], b );
    led_set_brightness( &strip[ RGB_G ], b / 2 );
    led_set_brightness( &strip[ RGB_R ], b / 2 );

    if ( state->t % ( PWM_MAX * 2 + 1 ) == ( PWM_MAX * 2 ) )
    {
        state->idx = ( state->idx + 1 ) % TOP_SIZE;
    }
    state->t++;
}


int main()
{
    for ( uint i = 0; i < SIZE( strip ); i++ )
    {
        led_init( &strip[ i ] );
    }

    uint pin_clk = 27,
         pin_dio = 26;

    TM1637_init( pin_clk, pin_dio );
    TM1637_clear();
    TM1637_set_brightness( 1 );

    crawling_state_t state = { 0 };
    alternate_state_t alt = { 0 };

    while ( true )
    {
        // crawling( &state );
        alternate( &alt );

        int ms = 20;
        // TODO: add speed change option
        sleep_ms( ms );
    }

    return 0;
}
