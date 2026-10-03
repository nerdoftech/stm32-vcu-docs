# Glossary

**BMS**
:   Battery management system. Measures cell voltages and temperatures and balances cells.

**CAN**
:   Controller Area Network, the two-wire bus most car modules use. The ZombieVerter has
    three CAN ports.

**CanMap**
:   The firmware feature that maps parameters to and from CAN frames without code.

**CCS / CHAdeMO**
:   DC fast-charging standards.

**Contactor**
:   A heavy-duty relay that connects the HV battery.

**CP / PP**
:   Control pilot and proximity pilot: the signal pins in an AC charge connector.

**DC-DC**
:   Converter that charges the 12 V battery from the HV pack.

**dir**
:   Selected direction: −1 reverse, 0 neutral, 1 drive, 2 park.

**GMLAN**
:   General Motors' in-vehicle CAN network (high-speed 500 kbit/s, low-speed 33.3 kbit/s
    single wire).

**HVIL**
:   High-voltage interlock loop: a low-voltage circuit through HV connectors that breaks
    when one is unplugged.

**ISA / IVT-S**
:   Isabellenhütte shunt current/voltage sensor with CAN output.

**LDU / SDU**
:   Tesla large / small drive unit.

**LIM**
:   BMW i3 Ladeinterfacemodul, the charge interface module that handles CCS and AC pilot.

**libopeninv / libopencm3**
:   Shared OpenInverter library and the STM32 peripheral library the firmware builds on.

**OI**
:   OpenInverter, Johannes Huebner's open-source inverter firmware and boards.

**opmode**
:   The VCU's operating mode: Off, Run, Precharge, PchFail, Charge, Preheat.

**Parameter / spot value**
:   A saved setting / a live read-only value.

**PDM**
:   Nissan Leaf power delivery module: AC charger plus DC-DC.

**potnom**
:   The processed throttle request, −100 to +100 %.

**Precharge**
:   Charging the inverter's capacitors through a resistor before closing the main
    contactor, to avoid a damaging inrush current.

**SDO**
:   Service data object, a CANopen-style request/response used to read and write
    parameters over CAN.

**Shunt**
:   A precision resistor used to measure current; here, a CAN-connected sensor that also
    measures voltage.

**SOC**
:   State of charge, percent.

**T15 / T30 / T50**
:   German terminal designations: ignition on, permanent battery, crank.

**udc / udcsw / udclim**
:   Measured DC voltage / voltage that ends precharge / over-voltage limit.

**VCU**
:   Vehicle control unit.
