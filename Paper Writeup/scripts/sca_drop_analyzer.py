"""
Parse OMNeT++ .sca files for RSU-side channel drop and application-layer
verification statistics across all scenarios.

Metrics reported (all at RSU side):
  Received       rsu[*].appl  rsuReceivedMsg:sum
  Verified       rsu[*].appl  rsuVerifiedMsg:sum
  Unverified     Received - Verified
  Prop drops     rsu[*].lteNic.phy  tbFailedDueToProp:sum
  HalfDuplex     rsu[*].lteNic.phy  tbFailedHalfDuplex:sum
  Unsensed       rsu[*].lteNic.phy  sciUnsensed:sum
  Interference   rsu[*].lteNic.phy  sciFailedDueToInterference:sum

Run from the "Paper Writeup" directory:
    python scripts/sca_drop_analyzer.py
"""

import re
from pathlib import Path
from collections import defaultdict

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
BASE_DIR = Path("Simulation Logs")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def scenario_number(name):
    m = re.search(r'^(\d+)_', name)
    return int(m.group(1)) if m else 0


def find_scenarios(base_dir):
    return sorted(
        (d for d in base_dir.iterdir()
         if d.is_dir() and (d / "results").is_dir()),
        key=lambda d: scenario_number(d.name)
    )


def find_sca(scenario_dir):
    scas = list((scenario_dir / "results").glob("*.sca"))
    return scas[0] if scas else None


def parse_sca(sca_path):
    """
    Parse an OMNeT++ .sca file and return two dicts restricted to RSU modules:
      appl_stats  {stat_base_name: value}   from rsu[*].appl
      phy_stats   {stat_base_name: value}   from rsu[*].lteNic.phy

    stat_base_name strips the trailing :sum / :mean suffix.
    Multiple RSU nodes are summed (there is usually only one).
    """
    appl  = defaultdict(float)
    phy   = defaultdict(float)

    rsu_appl_re = re.compile(r'^scalar\s+Highway\.rsu\[\d+\]\.appl\s+(\S+)\s+(.+)$')
    rsu_phy_re  = re.compile(r'^scalar\s+Highway\.rsu\[\d+\]\.lteNic\.phy\s+(\S+)\s+(.+)$')

    with open(sca_path, 'r', errors='replace') as fh:
        for line in fh:
            if not line.startswith('scalar '):
                continue

            m = rsu_appl_re.match(line.rstrip())
            if m:
                raw_name, raw_val = m.group(1), m.group(2)
                stat = raw_name.split(':')[0]
                try:
                    appl[stat] += float(raw_val)
                except ValueError:
                    pass
                continue

            m = rsu_phy_re.match(line.rstrip())
            if m:
                raw_name, raw_val = m.group(1), m.group(2)
                stat = raw_name.split(':')[0]
                try:
                    phy[stat] += float(raw_val)
                except ValueError:
                    pass

    return appl, phy


# ---------------------------------------------------------------------------
# Collect results
# ---------------------------------------------------------------------------
scenarios = find_scenarios(BASE_DIR)
if not scenarios:
    print(f"No scenarios found under {BASE_DIR}. Run from 'Paper Writeup' directory.")
    raise SystemExit(1)

print(f"Found {len(scenarios)} scenarios — parsing .sca files ...\n")

rows = []
for scenario_dir in scenarios:
    name = scenario_dir.name
    num  = scenario_number(name)
    sca  = find_sca(scenario_dir)

    if sca is None:
        rows.append((num, name, None))
        continue

    try:
        appl, phy = parse_sca(sca)

        received     = int(appl.get('rsuReceivedMsg', 0))
        verified     = int(appl.get('rsuVerifiedMsg',  0))
        unverified   = received - verified

        prop_drop    = int(phy.get('tbFailedDueToProp',          0))
        hd_drop      = int(phy.get('tbFailedHalfDuplex',         0))
        unsensed     = int(phy.get('sciUnsensed',                0))
        interference = int(phy.get('sciFailedDueToInterference', 0))

        rows.append((num, name, {
            'received':     received,
            'verified':     verified,
            'unverified':   unverified,
            'prop_drop':    prop_drop,
            'hd_drop':      hd_drop,
            'unsensed':     unsensed,
            'interference': interference,
        }))
    except Exception as e:
        print(f"  [ERROR] {name}: {e}")
        rows.append((num, name, None))


# ---------------------------------------------------------------------------
# Console table
# ---------------------------------------------------------------------------
cols = [
    ('#',           4,  'num',         'd'),
    ('Scenario',   26,  'name',        's'),
    ('Received',   10,  'received',    ',d'),
    ('Verified',   10,  'verified',    ',d'),
    ('Unverified',  11, 'unverified',  ',d'),
    ('Unverif%',    9,  '_pct',        's'),
    ('Prop drops',  11, 'prop_drop',   ',d'),
    ('HalfDuplex',  11, 'hd_drop',     ',d'),
    ('Unsensed',   10,  'unsensed',    ',d'),
    ('Interfrnce',  11, 'interference',',d'),
]

def header_line():
    parts = []
    for label, width, _, _ in cols:
        parts.append(f"{label:>{width}}")
    return ' | '.join(parts)

def sep_line():
    parts = []
    for _, width, _, _ in cols:
        parts.append('─' * width)
    return '─┼─'.join(parts)

def data_line(num, name, data):
    if data is None:
        pad = ' | '.join(' ' * w for _, w, _, _ in cols[2:])
        return f"{num:>4} | {name:<26} | {'NO SCA FILE':^{sum(w for _,w,_,_ in cols[2:]) + 3*(len(cols)-2)}}"

    pct = f"{100*data['unverified']/data['received']:.1f}%" if data['received'] else 'N/A'
    data['_pct'] = pct

    parts = []
    for label, width, key, fmt in cols:
        if key == 'num':
            parts.append(f"{num:>{width}}")
        elif key == 'name':
            parts.append(f"{name:<{width}}")
        else:
            val = data.get(key, '')
            if fmt == 's':
                parts.append(f"{val:>{width}}")
            else:
                parts.append(f"{val:{width}{fmt}}")
    return ' | '.join(parts)

total_width = sum(w for _, w, _, _ in cols) + 3 * (len(cols) - 1)

print('═' * total_width)
print(header_line())
print(sep_line())

prev_group = None
for num, name, data in rows:
    group = 'ECDSA' if 'ecdsa' in name.lower() else 'Falcon'
    if prev_group and group != prev_group:
        print('─' * total_width)
    prev_group = group
    print(data_line(num, name, data))

print('═' * total_width)
print()
print('Notes:')
print('  All metrics are at RSU[0] side only.')
print('  Unverified  = rsuReceivedMsg - rsuVerifiedMsg  (app layer)')
print('  Prop drops  = tbFailedDueToProp   (PHY: signal below SINR threshold)')
print('  HalfDuplex  = tbFailedHalfDuplex  (PHY: RSU was transmitting — expect 0)')
print('  Unsensed    = sciUnsensed          (PHY: control channel not detected)')
print('  Interfrnce  = sciFailedDueToInterference (PHY: SCI corrupted by interference)')
