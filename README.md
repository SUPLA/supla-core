# supla-core

> Part of **SUPLA** — an open smart home platform that brings together hardware manufacturers, the community, and users.  
> Learn more at https://www.supla.org

`supla-core` contains the **core server and system components** of the SUPLA smart home platform.

This repository includes the foundational services responsible for device communication, scheduling, and core system behavior.

---

## Scope of this repository

This repository focuses on **core platform components**.
It is not a complete, ready-to-run SUPLA installation and does not provide deployment or hosting instructions.

For related parts of the SUPLA ecosystem, see:

* Platform overview and architecture: [https://github.com/SUPLA](https://github.com/SUPLA)
* Production and self-hosted deployments: [https://github.com/SUPLA/supla-docker](https://github.com/SUPLA/supla-docker)
* Cloud web application and REST API: [https://github.com/SUPLA/supla-cloud](https://github.com/SUPLA/supla-cloud)
* Embedded device SDK: [https://github.com/SUPLA/supla-device](https://github.com/SUPLA/supla-device)

---

## Repository map

This repository contains multiple components maintained together. Use the list below as a navigation aid (not as full documentation):

* `supla-server/` - core network service responsible for device and client communication.
* `supla-scheduler/` - service responsible for executing schedules and time-based tasks.
* `supla-client/` - native client components and communication library used by SUPLA clients.
* `supla-console-client/` - command-line tool for basic control and diagnostics.
* `supla-common/` - shared code used across components, low-level communication, and protocol definitions.

---

## Development

This repository is primarily intended for:

* SUPLA core contributors,
* developers working on server-side and system-level components,
* contributors working on protocol, scheduling, or low-level communication.

This repository is **not** intended as a tutorial for setting up a production SUPLA system.

---

## Protocol

See [`how-protocol-works.md`](./how-protocol-works.md) for an overview of the SUPLA communication protocol.

---

## Contributing

Contributions are welcome.

Please read:

* [`CONTRIBUTING.md`](CONTRIBUTING.md)
* [`SECURITY.md`](SECURITY.md)

---

## License

This project is licensed under the **GPL-2.0** license.

