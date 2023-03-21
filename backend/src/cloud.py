class EVENT_ID:
  PING = 0x0
  SEMIHOURLY_REPORT = 0x1
  DAILY_REPORT = 0x2
  BAD_WEATHER = 0x3

EVENT_NAMES = {EVENT_ID.PING: "Ping", EVENT_ID.SEMIHOURLY_REPORT: "Semihourly report", EVENT_ID.DAILY_REPORT: "Daily report", EVENT_ID.BAD_WEATHER: "Breaking report"}

class DataFrame:
  temperature: float
  humidity: int

  def __init__(self, payload: bytes):
    temp_raw = int.from_bytes(payload[0:1], 'big') << 1
    self.humidity = int.from_bytes(payload[1:2], 'big')

    if self.humidity > 128:
      temp_raw += 1
      self.humidity -= 128

    self.temperature = temp_raw/10 - 5.5

class CONDITIONS_LIMITS:
  LOW_TEMP = 10
  HIGH_TEMP = 30
  LOW_HUM = 50
  HIGH_HUM = 70

  @classmethod
  def CheckTemp(cls, temp):
    if temp < cls.LOW_TEMP:
      return "(low)"
    elif temp > cls.HIGH_TEMP:
      return "(high)"
    return "(normal)"

  @classmethod
  def CheckHum(cls, hum):
    if hum < cls.LOW_HUM:
      return "(low)"
    elif hum > cls.HIGH_HUM:
      return "(high)"
    return "(normal)"

def happy_reporter(avg_data: DataFrame, high_data: DataFrame, low_data: DataFrame):
  placeholders = ["Average", "High", "Low"]
  dataframes = [avg_data, high_data, low_data]
  ret = ""
  for ph, df in zip(placeholders, dataframes):
    ret += f"{ph}:\n"
    ret += f"\tTemperature: {round(df.temperature, 1)}\n"
    ret += f"\tHumidity: {df.humidity}\n"

  return ret

def unhappy_reporter(avg_data: DataFrame, _, __):
  ret = f"\tTemperature: {round(avg_data.temperature, 1)} {CONDITIONS_LIMITS.CheckTemp(avg_data.temperature)}"
  ret += f"\n\tHumidity: {avg_data.humidity} {CONDITIONS_LIMITS.CheckHum(avg_data.humidity)}"
  return ret


REPORTERS = {EVENT_ID.PING: lambda _, __, ___ : "", EVENT_ID.SEMIHOURLY_REPORT: happy_reporter, EVENT_ID.DAILY_REPORT: happy_reporter, EVENT_ID.BAD_WEATHER: unhappy_reporter}

class Message:
  event_id: int # required
  avg_data: DataFrame # optional
  high_data: DataFrame # optional
  low_data: DataFrame # optional
  
  def __init__(self, event_id, period_data = None, high_data = None, low_data = None):
    self.event_id = event_id
    self.avg_data = period_data
    self.high_data = high_data
    self.low_data = low_data

  def __str__(self):
    return f"[{EVENT_NAMES[self.event_id]}]\n{REPORTERS[self.event_id](self.avg_data, self.high_data, self.low_data)}"

def handler(pd: "pipedream"):
  raw = bytes.fromhex(pd.steps["trigger"]["event"]["body"]["data"])

  msg = None
  event_id = int.from_bytes(raw[0:1], 'big')
  if event_id == EVENT_ID.PING:
    msg = Message(event_id)
  elif event_id == EVENT_ID.BAD_WEATHER:
    msg = Message(event_id, DataFrame(raw[1:3]))
  else:
    msg = Message(event_id, DataFrame(raw[1:3]), DataFrame(raw[3:5]), DataFrame(raw[5:7]))

  return {"message": str(msg)}
