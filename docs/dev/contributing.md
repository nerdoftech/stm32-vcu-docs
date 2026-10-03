# Contributing upstream

The firmware lives at <https://github.com/damienmaguire/Stm32-vcu>. Its own
`CONTRIBUTING.md` is the authority; this page summarises it and adds practical tips.

## Branches

- **`Vehicle_Testing`**: base most pull requests here. Expect to show some testing.
- **`Developmental_NOT_VEHICLE`**: for larger or framework changes that only need to
  compile and not break other features.
- **`master`**: validated releases. Do not target it unless a code owner asks.

Code owners decide what is accepted and may reject features that don't fit the
project. You can always keep a fork.

## Before opening a PR

- [ ] Branch from `Vehicle_Testing` (`git fetch origin Vehicle_Testing`).
- [ ] `make` succeeds and the size report still fits ([Memory budget](build.md#memory-budget)).
- [ ] `make Test && ./test/test_vcu` passes.
- [ ] `pre-commit run --all-files` leaves no changes (CI runs clang-format v20).
- [ ] New parameters use new IDs, and the "next id" comment is bumped.
- [ ] Option strings, enum values and `max` agree.
- [ ] You changed nothing in `libopeninv/` (send those changes to libopeninv upstream).
- [ ] Describe what you tested and on which vehicle/hardware.

## Reporting a bug

Upstream asks for:

1. Firmware version or build.
2. Your setup, ideally with pictures and diagrams.
3. Your parameters as a `.json` export.
4. What happens, with CAN logs or traces.
5. What you expected.

## Keeping a private fork current

If you carry your own classes (for example a vehicle class for your car), keep them in
new files and keep the edits to shared files (`param_prj.h`, `stm32_vcu.cpp`,
`Makefile`) as small as possible. Use option numbers and parameter IDs well above
upstream's so merges don't collide, then:

```bash
git remote add upstream https://github.com/damienmaguire/Stm32-vcu.git
git fetch upstream
git merge upstream/master        # or upstream/Vehicle_Testing
git submodule update --init --recursive
make clean && make
```
