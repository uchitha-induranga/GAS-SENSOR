# HRTE LPG Meter — domestic gas leak detector

A small Arduino device that clamps onto a household LPG cylinder, watches the air around the valve, and sounds an alarm the moment it picks up a leak. Live readings on a 16×2 LCD, buzzer and flashing backlight on alert.

📺 **[Full build guide on YouTube](https://www.youtube.com/watch?v=5oJ3uLvXpdg)**

![Device mounted on a cylinder](docs/device-on-cylinder.jpg)
![Alert state](docs/device-alert.jpg)

---

## Why I built it

Between November and mid-December 2021, Sri Lankan police recorded close to 730 gas stove explosion incidents. Kitchens, bakeries and restaurants across the country were affected, and the Court of Appeal eventually ordered a recall of cylinders.

The cause was publicly contested. The Consumer Affairs Authority raised concerns that the butane-to-propane ratio in domestic cylinders had shifted toward 50:50 from the usual 70:30, and warned that the resulting change in internal pressure could push gas past valves and fittings. The suppliers and the government denied any change had been made. A government expert committee later recommended keeping propane below 30% and increasing the ethyl mercaptan odorant so that leaks would be easier to smell.

Whatever the underlying cause, the practical problem for ordinary households was the same: gas was escaping from cylinder valves, often slowly, and often before anyone could smell it. Certified detectors were expensive and hard to find. So I built one that costs a few thousand rupees in parts and mounts directly on the cylinder.

---

## How it works

The MQ gas sensor's resistance drops when hydrocarbon gas reaches it, which the Arduino reads as a rising analog value. The firmware turns that raw number into a reliable alarm in four steps:

1. **Warm-up.** MQ sensors have a heater element and read high for the first minute or two after power-on. The unit counts down on the LCD instead of alarming during that window.
2. **Clean-air baseline.** After warm-up it averages 64 readings and stores that as the reference. The alarm point is set *relative to the baseline*, so the device adapts to its own sensor, the supply voltage and the room rather than trusting a hardcoded number.
3. **Averaged sampling.** Each measurement is the mean of 16 ADC reads, which removes most of the sensor noise.
4. **Confirmation and hysteresis.** The state only changes after three consecutive readings agree, and the clear threshold sits below the trip threshold. A single spike can't set off the buzzer, and a reading hovering at the boundary can't make it chatter on and off.

---

## Bill of materials

| Qty | Part | Notes |
|---|---|---|
| 1 | Arduino Uno | Any ATmega328P board works |
| 1 | MQ-4 gas sensor module | See the sensor note below — MQ-6 is the better choice |
| 1 | 16×2 LCD with I2C backpack | PCF8574, saves 6 pins over parallel wiring |
| 1 | Active buzzer, 5 V | Passive buzzers need `tone()` instead |
| 1 | 5 V USB supply | The MQ heater draws real current — don't run this from a weak phone charger |
| — | Enclosure, cable ties | Mine is a length of PVC pipe cut lengthwise |

### A note on the sensor

This build uses an **MQ-4**, which is tuned for methane and natural gas. LPG is mostly butane and propane. The MQ-4 does respond to it, which is why the device works, but an **MQ-6** is specifically characterised for LPG, butane and propane and gives noticeably better sensitivity to small leaks. **MQ-5** and **MQ-2** are also reasonable.

The modules share the same 4-pin pinout, so swapping one in needs no code or wiring changes — only a fresh baseline, which the firmware captures on its own at every power-up.

---

## Wiring

| From | To |
|---|---|
| MQ sensor `VCC` | Uno `5V` |
| MQ sensor `GND` | Uno `GND` |
| MQ sensor `A0` (analog out) | Uno `A0` |
| LCD backpack `VCC` | Uno `5V` |
| LCD backpack `GND` | Uno `GND` |
| LCD backpack `SDA` | Uno `A4` |
| LCD backpack `SCL` | Uno `A5` |
| Buzzer `+` | Uno `D2` |
| Buzzer `−` | Uno `GND` |

The sensor module's digital output `D0` is not used — the firmware reads the analog pin so it can show a live number and set its own threshold.

![Wiring diagram](docs/wiring-diagram.png)

---

## Setup

1. Install the **LiquidCrystal_I2C** library through the Arduino Library Manager.
2. Open `hrte_lpg_meter/hrte_lpg_meter.ino`, select **Arduino Uno**, and upload.
3. If the screen lights up but stays blank, your backpack is on a different I2C address. Run any I2C scanner sketch, then change `LCD_ADDRESS` at the top of the file. `0x27` and `0x3F` are the two most common; this unit uses `0x26`.
4. Leave a brand-new MQ sensor powered for **24 to 48 hours** before trusting it. This burn-in is specified by the manufacturer, and readings drift a lot until it's done.

---

## Calibration

Open the Serial Monitor at 9600 baud. The sketch prints the baseline and thresholds at startup, then streams `value,alarmState` continuously — you can paste that into a spreadsheet and plot it.

| Setting | What it does |
|---|---|
| `ALARM_MARGIN` | Counts above the clean-air baseline that trigger the alarm. Lower it for more sensitivity, raise it if you get false alerts. |
| `CLEAR_MARGIN` | Must be lower than `ALARM_MARGIN`. The gap between them is the hysteresis band. |
| `CONFIRM_COUNT` | Readings that must agree before the state flips. Raise it if brief kitchen fumes trip the alarm. |
| `WARMUP_SECONDS` | Extend it if readings are still falling when the baseline is captured. |

To find your own margin: run the device in clean air and note the steady value, then hold an unlit gas lighter near the sensor and press the valve briefly. The difference between the two tells you what margin actually separates "leak" from "normal".

**Important:** the baseline is captured at power-up. If you switch the unit on in a room that already has gas in it, the baseline will be wrong and the alarm will not fire. Always power it up in fresh air.

---

## Safety notice

Please read this before relying on the device.

This is a **hobby early-warning aid, not a certified gas detector.** It has no third-party approval, no self-test, and no fail-safe if the sensor dies quietly. MQ sensors drift with age, humidity and temperature, and they respond to alcohol, solvents, smoke and cooking fumes as well as to LPG.

- Do not treat it as a replacement for a certified LPG detector, and do not treat a silent buzzer as proof there is no leak.
- Check the valve and regulator with soapy water — bubbles are still the most reliable home test there is.
- If you smell gas, act on your nose. Shut the valve, ventilate, and do not switch anything electrical on or off.
- The electronics are not intrinsically safe. Mount the unit near the cylinder but not directly over the valve outlet, and use a mains adapter, not batteries taped to the cylinder.
- LPG is heavier than air and pools low, so mount the sensor low. Never seal it inside an airtight box — it needs airflow to work at all.

---

## What changed from the first version

The version in the video worked, and this repo keeps it in `legacy/` for reference. The current firmware fixes the following:

| Issue | Fix |
|---|---|
| Fixed threshold of `smoke > 300` — meaningless across different sensors and supply voltages | Threshold is now set relative to a clean-air baseline captured at every power-up |
| No warm-up, so the device alarmed on its own for the first minute after switch-on | 90-second countdown before the first reading is taken |
| Single `analogRead()` per cycle, so sensor noise reached the alarm logic directly | Each measurement averages 16 samples |
| No hysteresis — a reading sitting near 300 made the buzzer chatter | Separate trip and clear thresholds |
| No confirmation — one spike triggered a full alert | Three consecutive readings must agree before the state changes |
| `lcd.clear()` on every loop caused visible flicker | Padded 16-character lines overwrite in place |
| `delay(500)` inside the alert branch blocked everything, including further sensor reads | Buzzer and backlight pulse from `millis()`; nothing blocks |
| `Serial.begin(9600)` was called but nothing was ever printed | Streams readings and alarm state for calibration |
| Magic numbers scattered through the code | Named constants in one configuration block |

---

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| Backlight on, no text | Wrong I2C address, or the contrast pot on the backpack needs turning |
| Nothing at all on the LCD | SDA and SCL swapped — SDA is `A4`, SCL is `A5` |
| Alarm fires immediately at power-up | Not warmed up, or powered on in air that already contains gas |
| Reading never moves | Sensor `A0` not connected, or you wired the module's `D0` pin by mistake |
| Alarm never fires even near gas | Margin too high, or an MQ-4 that's simply not sensitive enough for LPG — try an MQ-6 |
| Readings jump around wildly | Undersized power supply; the MQ heater needs a few hundred mA |

---

## Credits

Designed, built and documented by **Uchitha Induranga** — *HRTE Projects*.

## License

MIT — see [LICENSE](LICENSE).
