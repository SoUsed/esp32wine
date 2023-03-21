import unittest
from unittest.mock import MagicMock
from cloud import handler


class TestHandler(unittest.TestCase):
    
    def setUp(self):
        self.pd = MagicMock()
        self.pd.steps = {
            "trigger": {
                "event": {
                    "body": {
                        "data": "01000a805e"
                    }
                }
            }
        }
    
    def test_ping_event(self):
        self.pd.steps["trigger"]["event"]["body"]["data"] = "0000"
        result = handler(self.pd)
        expected_output = "[Ping]\n"
        self.assertEqual(result["message"], expected_output)
    
    def test_semihourly_report_event(self):
        self.pd.steps["trigger"]["event"]["body"]["data"]   = "0180a2"
        result = handler(self.pd)
        expected_output = "[Semihourly report]\nAverage:\n\tTemperature: 20.2\n\tHumidity: 34\nHigh:\n\tTemperature: -5.5\n\tHumidity: 0\nLow:\n\tTemperature: -5.5\n\tHumidity: 0\n"
        self.assertEqual(result["message"], expected_output)
    
    def test_daily_report_event(self):
        self.pd.steps["trigger"]["event"]["body"]["data"] = "0280a280a2a0"
        result = handler(self.pd)
        expected_output = "[Daily report]\nAverage:\n\tTemperature: 20.2\n\tHumidity: 34\nHigh:\n\tTemperature: 20.2\n\tHumidity: 34\nLow:\n\tTemperature: 26.5\n\tHumidity: 0\n"
        self.assertEqual(result["message"], expected_output)
    
    def test_bad_weather_event(self):
        self.pd.steps["trigger"]["event"]["body"]["data"] = "030014"
        result = handler(self.pd)
        expected_output = "[Breaking report]\n\tTemperature: -5.5 (low)\n\tHumidity: 20 (low)"
        self.assertEqual(result["message"], expected_output)

        self.pd.steps["trigger"]["event"]["body"]["data"] = "0300c5"
        result = handler(self.pd)
        expected_output = "[Breaking report]\n\tTemperature: -5.4 (low)\n\tHumidity: 69 (normal)"
        self.assertEqual(result["message"], expected_output)
    
if __name__ == '__main__':
    unittest.main()
