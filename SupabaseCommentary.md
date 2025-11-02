# QA & Telemetry Commentary

## Overview
Collection of performance metrics in Unreal using Supabase.
The system consists of three integrated parts:
1. **Google Form** for user feedback and consent  
2. **C++ Telemetry Subsystem** in Unreal Engine  
3. **Supabase** backend for cloud storage and analysis  

---
## Feedback and Consent
Consent and player opinions are gathered via a Google Form *“FMP Racing Prototype – QA Survey.”*  
The form asked participants to confirm consent and provide:
- Device used (Desktop / Laptop / Other)  
- Ratings for difficulty, enjoyment, and clarity of audio/visuals  
- Comments on bugs and suggestions  


T## Telemetry Subsystem
A  **C++ system** called `UPlaytestTelemetrySubsystem` was implemented inside the Unreal project to record gameplay performance.  
It runs automatically at runtime and sends structured JSON data to Supabase using Unreal’s `FHttpModule`.

### Core functionality
- Records and uploads:
  - **Average FPS**  
  - **Minimum FPS**  
  - **Maximum FPS**  
  - **Session duration**  
  - **Build version and device info**
- Logs these values once per second during gameplay.
- Posts all telemetry as JSON through HTTPS to Supabase tables.

A `TelemetryStartActor` Blueprint triggers the subsystem at game start.  
When a player begins, it automatically creates a new Supabase session with:
```cpp
Telemetry->StartSession(true, TEXT("Windows Desktop"));
```

## Supabase Backend

Supabase was used to store and manage telemetry data using its built-in REST API. The Unreal telemetry subsystem authenticates each HTTP request using Supabase’s service role API key, included securely in the project’s configuration file.

### Tables

playtest_sessions — one entry per play session

Columns: id, created_at, device, build, consent, seconds_played

playtest_events — individual telemetry logs

Columns: session_id, type, fields (JSON), created_at

A SQL view summarised performance for each session:

This provided a clean summary for analysis and CSV export.

## Edge Functions

Two Supabase Edge Functions were created to handle incoming data securely:

insert_session – creates a new row in the playtest_sessions table when a player begins testing.

insert_event – records performance metrics (average, min, and max FPS) for that active session.

These functions replace direct table writes from Unreal and provide a secure, modular interface for logging telemetry data over HTTPS.

## Data Analysis and Visualisation

Exported data from Supabase was analysed using Python (Pandas + Matplotlib).
A script (plot_fps.py) produced clear graphs comparing average, minimum, and maximum FPS for each playtest session.

Example output:

<img width="1500" height="750" alt="vw_session_perf_rows_by_session" src="https://github.com/user-attachments/assets/c3c09677-c357-4b25-af95-70cb010b67de" />

## Tools and Technologies

Unreal Engine 5.4 (C++) — for telemetry and in-game actor logic

Supabase (PostgreSQL + REST API) — for secure data storage

Google Forms — for consent and player feedback

Python (Pandas, Matplotlib) — for data analysis and visualisation

## Impact

FPS telemetry provides objective performance evidence.

Cloud storage allows easy comparison between builds and devices.

Combining ** telemetry (FPS, duration)** with **feedback (surveys, comments)** gives a clearer picture of how the game performs and feels.

