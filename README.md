# stm32-vcu-docs

Claude-generated documentation for the [ZombieVerter VCU firmware](https://github.com/damienmaguire/Stm32-vcu)
(damienmaguire/Stm32-vcu), built with [MkDocs Material](https://squidfunk.github.io/mkdocs-material/).

The site has three parts:

- **User Guide**: wiring, first setup, throttle, charging, BMS and shunt, GPIO, CAN mapping and troubleshooting.
- **Developer Guide**: build, architecture, state machine, parameters, CAN, I/O, module interfaces, adding a vehicle, and a GM GMT900 worked example.
- **Reference**: every parameter and spot value (generated from the firmware source), enums, CAN IDs by module, pinout and a glossary.

Written against firmware V2.41A (commit `b061f84`).

## Build locally

```bash
pip install -r requirements.txt
mkdocs serve          # http://127.0.0.1:8000
mkdocs build --strict # what CI runs
```

## Regenerate the parameter reference

```bash
git clone --recurse-submodules https://github.com/damienmaguire/Stm32-vcu.git /tmp/Stm32-vcu
python3 scripts/gen_param_reference.py /tmp/Stm32-vcu
```

Descriptions live in `scripts/param_descriptions.py`.

## Example code

`examples/gmt900/` holds a compile-tested `GM_GMT900` vehicle class skeleton and a patch
that registers it in the firmware. See the GMT900 page in the Developer Guide.

## Publishing

`.github/workflows/docs.yml` builds on every PR and deploys to the `gh-pages` branch on
pushes to `main`. Enable GitHub Pages for this repo with source set to the `gh-pages` branch.
