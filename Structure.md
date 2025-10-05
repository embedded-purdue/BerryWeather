# Task Structure

> **Legend**
> 🔵 Setup (run once at start)
> 🔴 Occasional / Periodic (runs once in a while)
> 🟢 Continuous (always after setup)

---

## System Overview

Two cooperating nodes:

* **Satellite** — collects sensor data and sends summarized packets via LoRa at intervals.
* **Middle Man** — listens for LoRa packets and forwards data to an MQTT broker.

Visual map of tasks and lifecycles below (colors follow the legend).

```mermaid
flowchart TD
  %% SATELLITE
  subgraph Satellite
    A[Setup_comm]:::blue --> R{{Runtime}}
    R --> C1[Coll_temp]:::green
    R --> C2[Coll_humidity]:::green
    R --> C3[coll_soilTemp]:::green
    R --> C4[coll_soilMoisture]:::green
    R --> S1[Send_loraMsg (every ~30 min)]:::red
    A --> T1[Send_test]:::red
  end

  %% MIDDLE MAN
  subgraph Middle_Man
    M1[Setup_comm]:::blue --> M2[Setup_mqtt]:::blue --> MRun{{Runtime}}
    MRun --> L1[Listen]:::green
    MRun --> LT[Listen_test]:::red
    L1 -- new packet --> M3[Send_mqtt (state updates)]:::red
  end

  classDef blue fill:#1f77b4,stroke:#0e4166,color:#fff;
  classDef red fill:#d62728,stroke:#7f1d1d,color:#fff;
  classDef green fill:#2ca02c,stroke:#145214,color:#fff;
```

---

## Satellite Tasks

| Task                   | Phase         | When               |     Priority (suggested) | Details                                                                                                  |
| ---------------------- | ------------- | ------------------ | -----------------------: | -------------------------------------------------------------------------------------------------------- |
| 🔵 `Setup_comm`        | Setup         | Boot               |     **High** (init-only) | Configure UART → LoRa module; send required AT cmds; confirm module state before enabling runtime tasks. |
| 🔴 `Send_test`         | Bring-up / QA | As needed          |                   Medium | Send a simple "Hello" frame to validate link to Middle Man. Disable in production builds.                |
| 🔴 `Send_loraMsg`      | Periodic      | ~ every **30 min** | **Higher than `coll_X`** | Packages latest sensor readings and transmits via LoRa. Preempts collection tasks if needed.             |
| 🟢 `Coll_temp`         | Continuous    | Post-setup         |                      Low | Periodically sample temperature sensor; buffer or average.                                               |
| 🟢 `Coll_humidity`     | Continuous    | Post-setup         |                      Low | Periodically sample humidity sensor; buffer or average.                                                  |
| 🟢 `coll_soilTemp`     | Continuous    | Post-setup         |                      Low | Read soil temperature sensor; buffer or average.                                                         |
| 🟢 `coll_soilMoisture` | Continuous    | Post-setup         |                      Low | Read soil moisture sensor; buffer or average.                                                            |

> **Notes**
>
> * `Send_loraMsg` has higher priority than the `coll_X` tasks to ensure timely transmissions.
> * Consider a shared **Data Buffer** (ring buffer or struct) populated by `coll_X` tasks and consumed by `Send_loraMsg`.

---

## Middle Man Tasks

| Task                   | Phase         | When          | Priority (suggested) | Details                                                                                                                       |
| ---------------------- | ------------- | ------------- | -------------------: | ----------------------------------------------------------------------------------------------------------------------------- |
| 🔵 `Setup_comm`        | Setup         | Boot          |     High (init-only) | Configure UART/LoRa receive path; verify module readiness.                                                                    |
| 🔵 `Setup_mqtt`        | Setup         | Boot (once)   |               Medium | Publish **discovery topics** (Retain = **true**, QoS = **1**). Run again only if adding sensors or changing discovery schema. |
| 🔴 `Listen_test`       | Bring-up / QA | As needed     |               Medium | Validate reception path by waiting for "Hello" and logging it.                                                                |
| 🟢 `Listen`            | Continuous    | Always        |             **High** | Block/wait on LoRa receive; on message, push payload to a queue/event for `Send_mqtt`.                                        |
| 🔴 `Send_mqtt` (state) | Event-driven  | On new packet |          Medium–High | Forward parsed sensor data to MQTT **state topics** (Retain = **false**, QoS = **0/1** as needed).                            |

> **Notes**
>
> * Keep `Listen` responsive (do not do heavy parsing on the RX task). Parse/publish in `Send_mqtt` after queueing the payload.
> * Re-run `Setup_mqtt` only when discovery schema changes.

---

## MQTT Topics (Middle Man)

| Topic Type    |    Retain |    QoS | Produced By  | When                                  |
| ------------- | --------: | -----: | ------------ | ------------------------------------- |
| **Discovery** |  **true** |  **1** | `Setup_mqtt` | Once at boot (or when schema changes) |
| **State**     | **false** | 0 or 1 | `Send_mqtt`  | On each received LoRa packet          |

> Tip: Namespace topics per device (e.g., `sensors/<sat_id>/temp`, `.../humidity`, `.../soil_temp`, `.../soil_moisture`).

---

## Scheduling & Coordination Hints

* **Priorities (example FreeRTOS-style):**

  * High: `Listen`, `Send_loraMsg`, `Setup_comm` (during init only)
  * Medium: `Setup_mqtt`, `Send_mqtt`, `Send_test`
  * Low: all `coll_X`
* **Timing:**

  * `Send_loraMsg` every ~30 minutes (use a software timer or `vTaskDelayUntil`).
  * `coll_X` sample intervals may be shorter (e.g., 1–5 minutes) and averaged before transmit.
* **Queues/Events:**

  * `Listen → Send_mqtt`: RX queue with a max payload struct (timestamp, IDs, sensor bundle).
  * `coll_X → Send_loraMsg`: Shared buffer guarded by a mutex or lockless snapshot if small.

---

## Ready-to-Use Checklists

### Satellite Startup (🔵)

* [ ] Init clocks, UART, GPIO, sensors
* [ ] `Setup_comm` → LoRa ready
* [ ] Enable `coll_X` tasks (🟢)
* [ ] Start periodic timer for `Send_loraMsg` (🔴)

### Middle Man Startup (🔵)

* [ ] Init UART/LoRa RX path
* [ ] `Setup_mqtt` publishes discovery topics (retain ✅, QoS 1)
* [ ] Start `Listen` (🟢) and ensure RX queue is draining to `Send_mqtt` (🔴)

---

## File/Module Hints (Optional)

```
/firmware
  /satellite
    tasks_setup_comm.c
    task_send_test.c
    task_send_lora.c
    task_coll_temp.c
    task_coll_humidity.c
    task_coll_soiltemp.c
    task_coll_soilmoist.c
    sensors/*.c
    lora/*.c
  /middleman
    tasks_setup_comm.c
    task_setup_mqtt.c
    task_listen.c
    task_send_mqtt.c
    mqtt/*.c
    lora/*.c
  /common
    proto_packets.h/.c
    sensors_shared.h/.c
    config.h
```

> Keep names 1:1 with task labels for clarity. Each task file exposes `void vTaskName(void* pvParameters)` and internal helpers.
