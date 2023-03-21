# Pipedream Cloud Backend Data Flow
## Input data format (binary)
### Overall structure
[EVENT_ID | 1B]\[DFRAME | 2B]{0,3}
### EVENT_IDs
0x0 | EVENT_ID.PING                                 - connectivity test (used only for test cases, don't contain any DataFrames)
0x1 | EVENT_ID.SEMIHOURLY_REPORT - 30min report (contains 3 DataFrames)
0x2 | EVENT_ID.DAILY_REPORT               - daily report (contains 3 DataFrames)
0x3 | EVENT_ID.BAD_WEATHER             - breaking report on bad condition (contains 1 DataFrame)
### DFRAME Structure
DFRAME is 16bit integer:
	* 9 bits (MSB) - temperature (0-511 => t/10 - 5.5: range is [-5.5, 45.6] with step 0.1
					Example: 101000111 (27.2)
	* 7 bits (LSB) - humidity (0-127 => h: range is [0,100] with step 1
				  Example: 1000011 (67)
So the example DFRAME is 1010001111000011 => 0xA3C3