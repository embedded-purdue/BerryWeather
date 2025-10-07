# Task Structure

> **Legend**
> 🔵 Setup (run once at start)
> 🔴 Occasional / Periodic (runs once in a while)
> 🟢 Continuous (always after setup)

---

## Satellite Tasks

* 🔵 **Setup_comm** (Priority: **High**): Configure UART → LoRa module. Initialize at boot.
* 🔵 **Send_test** (Priority: **Medium**): Sends “Hello” to Middle Man (debug/test only).
* 🔴 **Send_loraMsg** (Priority: **High**): Every ~30 minutes. Packages and sends collected sensor data. **Higher priority than `coll_X` tasks**.
* 🟢 **Coll_temp** (Priority: **Low**): Collects temperature readings continuously.
* 🟢 **Coll_humidity** (Priority: **Low**): Collects humidity readings continuously.
* 🟢 **coll_soilTemp** (Priority: **Low**): Collects soil temperature readings continuously.
* 🟢 **coll_soilMoisture** (Priority: **Low**): Collects soil moisture readings continuously.

---

## Middle Man Tasks

* 🔵 **Setup_comm** (Priority: **High**): Configure UART/LoRa RX path. Initialize at boot.
* 🔵 **Setup_mqtt** (Priority: **Medium**): Publish discovery topics (retain = true, QoS = 1). Runs once at startup or when schema changes.
* 🔵 **Listen_test** (Priority: **Medium**): Waits for test “Hello” from Satellite.
* 🟢 **Listen** (Priority: **High**): Continuously waits for LoRa packets. Always running.
* 🔴 **Send_mqtt** (Priority: **Medium–High**): Triggered when new LoRa packet arrives. Publishes state data to MQTT (retain = false, QoS = 0/1).

---

## Notes

* **Satellite:** `Send_loraMsg` > `coll_X` tasks in priority.
* **Middle Man:** `Listen` must be responsive. 
* **MQTT:**

  * Discovery topics → retain = true, QoS = 1.
  * State topics → retain = false, QoS = 0/1.
