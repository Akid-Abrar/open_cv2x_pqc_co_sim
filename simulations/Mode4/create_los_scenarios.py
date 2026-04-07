#!/usr/bin/env python3
"""Generate SUMO route, sumocfg, and launchd files for LOS A-F scenarios."""

import os

BASE_DIR = os.path.join(os.path.dirname(__file__), "intersections")

# LOS level -> (density veh/km, flow veh/h per approach, description)
# speed = 20.12 m/s = 72.43 km/h, flow = density * speed
LOS_LEVELS = {
    "A": (10,   724,  "Free-flow"),
    "B": (20,  1448,  "Reasonably free flow"),
    "C": (30,  2172,  "Stable flow"),
    "D": (40,  2896,  "Approaching unstable"),
    "E": (50,  3620,  "At capacity"),
    "F": (100, 7240,  "Oversaturated / breakdown"),
}

FLOW_END = 400   # seconds — vehicles generated for this long
SUMO_END = 500   # SUMO simulation end time

ROUTE_TEMPLATE = """\
<routes>
    <vType id="car" accel="2.6" decel="4.5" length="5" maxSpeed="20.12"/>

    <!-- From North: straight, left, right -->
    <route id="route_N_S" edges="N_to_C C_to_S"/>
    <route id="route_N_E" edges="N_to_C C_to_E"/>
    <route id="route_N_W" edges="N_to_C C_to_W"/>

    <!-- From South: straight, left, right -->
    <route id="route_S_N" edges="S_to_C C_to_N"/>
    <route id="route_S_W" edges="S_to_C C_to_W"/>
    <route id="route_S_E" edges="S_to_C C_to_E"/>

    <!-- From East: straight, left, right -->
    <route id="route_E_W" edges="E_to_C C_to_W"/>
    <route id="route_E_S" edges="E_to_C C_to_S"/>
    <route id="route_E_N" edges="E_to_C C_to_N"/>

    <!-- From West: straight, left, right -->
    <route id="route_W_E" edges="W_to_C C_to_E"/>
    <route id="route_W_N" edges="W_to_C C_to_N"/>
    <route id="route_W_S" edges="W_to_C C_to_S"/>

    <!-- LOS {los}: {desc} — {density} veh/km, {flow} veh/h per approach -->
    <!-- 80/10/10 split: through={through}, left={left}, right={right} veh/h -->
{flows}
</routes>
"""

FLOW_BLOCK = """\
    <!-- {direction} approach: {flow} veh/h -->
    <flow id="flow_{d1}_{d2}" type="car" route="route_{d1}_{d2}"
          begin="0" end="{end}" vehsPerHour="{through}" departLane="random" departPos="random_free"/>
    <flow id="flow_{d1}_{d3}" type="car" route="route_{d1}_{d3}"
          begin="0" end="{end}" vehsPerHour="{left}" departLane="random" departPos="random_free"/>
    <flow id="flow_{d1}_{d4}" type="car" route="route_{d1}_{d4}"
          begin="0" end="{end}" vehsPerHour="{right}" departLane="random" departPos="random_free"/>
"""

SUMOCFG_TEMPLATE = """\
<configuration>
    <input>
        <net-file value="intersection.net.xml"/>
        <route-files value="intersection_LOS_{los}.rou.xml"/>
    </input>
    <time>
        <begin value="0"/>
        <end value="{sumo_end}"/>
        <step-length value="0.1"/>
    </time>
    <report>
        <xml-validation value="never"/>
        <xml-validation.net value="never"/>
        <no-step-log value="true"/>
    </report>
    <gui_only>
        <start value="false"/>
    </gui_only>
</configuration>
"""

LAUNCHD_TEMPLATE = """\
<launch>
   <basedir path="/home/veins/src/simulte/simulations/Mode4/intersections"/>
    <copy file="intersection.net.xml"/>
    <copy file="intersection_LOS_{los}.rou.xml"/>
    <copy file="intersection_LOS_{los}.sumocfg" type="config"/>
</launch>
"""

# Approach definitions: (direction_name, straight, left, right)
APPROACHES = [
    ("North", "N", "S", "E", "W"),
    ("South", "S", "N", "W", "E"),
    ("East",  "E", "W", "S", "N"),
    ("West",  "W", "E", "N", "S"),
]

for los, (density, flow, desc) in LOS_LEVELS.items():
    through = int(round(flow * 0.8))
    left    = int(round(flow * 0.1))
    right   = int(round(flow * 0.1))

    # Build flow blocks for all 4 approaches
    flows = ""
    for direction, d1, d2, d3, d4 in APPROACHES:
        flows += FLOW_BLOCK.format(
            direction=direction, d1=d1, d2=d2, d3=d3, d4=d4,
            flow=flow, through=through, left=left, right=right,
            end=FLOW_END
        )

    # Write route file
    route_content = ROUTE_TEMPLATE.format(
        los=los, desc=desc, density=density, flow=flow,
        through=through, left=left, right=right, flows=flows
    )
    route_path = os.path.join(BASE_DIR, f"intersection_LOS_{los}.rou.xml")
    with open(route_path, "w") as f:
        f.write(route_content)
    print(f"Created {route_path}")

    # Write sumocfg file
    sumocfg_content = SUMOCFG_TEMPLATE.format(los=los, sumo_end=SUMO_END)
    sumocfg_path = os.path.join(BASE_DIR, f"intersection_LOS_{los}.sumocfg")
    with open(sumocfg_path, "w") as f:
        f.write(sumocfg_content)
    print(f"Created {sumocfg_path}")

    # Write launchd file
    launchd_content = LAUNCHD_TEMPLATE.format(los=los)
    launchd_path = os.path.join(BASE_DIR, f"intersection_LOS_{los}.launchd.xml")
    with open(launchd_path, "w") as f:
        f.write(launchd_content)
    print(f"Created {launchd_path}")

print("\nDone! Created 6 × 3 = 18 files.")
