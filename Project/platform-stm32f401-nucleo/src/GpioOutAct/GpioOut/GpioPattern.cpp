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
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 2960}
			}
		},
		// Pattern 1 - Power Level 2
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 5960}
			}
		},
		// Pattern 2 - Power Level 3
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 8960}
			}
		},
		// Pattern 3 - Power Level 4
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 11960}
			}
		},
		// Pattern 4 - Power Level 5
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 14960}
			}
		},
		// Pattern 5 - Power Level 6
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 17960}
			}
		},
		// Pattern 6 - Power Level 7
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 20960}
			}
		},
		// Pattern 7 - Power Level 8
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 23960}
			}
		},
		// Pattern 8 - Power Level 9
		{5,
			{
				{200, 10}, {400, 10}, {600, 10}, {800, 10}, {1000, 26960}
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
