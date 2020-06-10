#include "GpioPattern.h"

namespace APP {

// Illustration only.
/*
GpioInterval testInterval = {1, 2};

GpioPattern testPattern = {
    2, {{1,2}, {2, 3}}
};
*/

GpioPatternSet const TEST_GPIO_PATTERN_SET = {
    2, 
    {
        // Pattern 0
        {21, 
            {
                {10,50}, {20,50}, {30,50}, {50,50}, {70,50}, {100,50}, {200,50}, {400,50}, {500,50}, {900,50},  // ramp up
                {900, 2000},                                                                                    // constant high
                {900,50}, {500,50}, {400,50}, {200,50}, {100,50}, {70,50}, {50,50}, {30,50}, {20,50}, {10,50},  // ramp down          
            }
        },
        // Pattern 1
        {4, 
            {
                {500,200}, {0, 200}, {500, 200}, {0, 1000}      // two short blinks.
            }
        }
    }
};

GpioPatternSet const MAGNETRON_PATTERN_SET = {
	10,
	{
		// Pattern 0 - Power Level 1
		{5,
			{
				{20, 10}, {40, 10}, {60, 10}, {80, 10}, {100, 2960}
			}
		},
		// Pattern 1 - Power Level 2
		{5,
			{
				{40, 10}, {80, 10}, {120, 10}, {160, 10}, {200, 5960}
			}
		},
		// Pattern 2 - Power Level 3
		{5,
			{
				{60, 10}, {120, 10}, {180, 10}, {240, 10}, {300, 8960}
			}
		},
		// Pattern 3 - Power Level 4
		{5,
			{
				{80, 10}, {160, 10}, {240, 10}, {320, 10}, {400, 11960}
			}
		},
		// Pattern 4 - Power Level 5
		{5,
			{
				{100, 10}, {200, 10}, {300, 10}, {400, 10}, {500, 14960}
			}
		},
		// Pattern 5 - Power Level 6
		{5,
			{
				{120, 10}, {240, 10}, {360, 10}, {480, 10}, {600, 17960}
			}
		},
		// Pattern 6 - Power Level 7
		{5,
			{
				{140, 10}, {280, 10}, {420, 10}, {560, 10}, {700, 20960}
			}
		},
		// Pattern 7 - Power Level 8
		{5,
			{
				{160, 10}, {320, 10}, {480, 10}, {640, 10}, {800, 23960}
			}
		},
		// Pattern 8 - Power Level 9
		{5,
			{
				{180, 10}, {360, 10}, {540, 10}, {720, 10}, {900, 26960}
			}
		},
		// Pattern 9 - Power Level 10
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 29960}
			}
		}
	}
};

} // namespace APP
