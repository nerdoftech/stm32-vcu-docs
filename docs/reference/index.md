# Reference

| Page | Contents |
|---|---|
| [Parameters](parameters.md) | Every setting: range, default, persistent ID and meaning, grouped as in the web interface. Generated from the firmware source. |
| [Spot values](spot-values.md) | Every read-only value the firmware publishes. Generated. |
| [Enumerations](enums.md) | Option lists for enumerated parameters. Generated. |
| [CAN IDs by module](can-ids.md) | Which CAN IDs each driver receives and sends. |
| [Connector pinout](pinout.md) | The 56-pin connector. |
| [Glossary](glossary.md) | Terms and abbreviations. |

## Regenerating the generated pages

```bash
pip install -r requirements.txt
git clone --recurse-submodules https://github.com/damienmaguire/Stm32-vcu.git /tmp/Stm32-vcu
python3 scripts/gen_param_reference.py /tmp/Stm32-vcu
mkdocs serve
```

Descriptions are kept in `scripts/param_descriptions.py`. Anything new in the firmware
shows up as "(undocumented)" until a description is added.
