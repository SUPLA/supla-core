# Contributing to SUPLA Core

Thank you for your interest in contributing to **SUPLA Core**.
This document describes the rules and guidelines for contributing code,
documentation, and ideas to the project.

## Table of contents

* [Contributor License Agreement (CLA)](#contributor-license-agreement-cla)
* [Project scope and goals](#project-scope-and-goals)
* [Reporting issues](#reporting-issues)
* [Feature requests](#feature-requests)
* [Development guidelines](#development-guidelines)
* [Pull request process](#pull-request-process)
* [Licensing](#licensing)

---

## Contributor License Agreement (CLA)

This project requires acceptance of a **Contributor License Agreement (CLA)**.

Before a pull request can be merged, the CLA must be accepted by the contributor.

Pull requests with an unaccepted CLA will be blocked automatically.

---

## Project scope and goals

`supla-core` contains the **core server and system components** of the SUPLA platform.

Key goals:

* stability and reliability of core services,
* backward compatibility of protocols where possible,
* predictable behavior for both official and self-hosted deployments,
* long-term maintainability.

This repository focuses on low-level and system-critical components.
Changes should be made with special care for compatibility, security, and operational impact.

---

## Reporting issues

Before opening an issue:

1. Search existing issues (open and closed).
2. Verify that the issue applies to the core components of the platform.

When reporting a bug, please include:

* affected component (e.g. server, scheduler, client library, tools),
* SUPLA Core version (release tag or commit hash),
* operating system and architecture,
* steps to reproduce the issue,
* expected and actual behavior,
* relevant logs (with secrets removed).

If the issue depends on deployment or containerization, it may belong to:

* [https://github.com/SUPLA/supla-docker](https://github.com/SUPLA/supla-docker)

Security-related issues must not be reported via public issues.
See `SECURITY.md`.

---

## Feature requests

Feature requests are welcome.

Please describe:

* the problem you are trying to solve,
* why existing functionality is insufficient,
* potential impact on protocol compatibility or existing deployments,
* whether the change affects public or documented behavior.

Large or protocol-affecting changes should be discussed before implementation.

---

## Development guidelines

* Keep changes focused and minimal.
* Avoid breaking protocol or behavioral compatibility without prior discussion.
* Be explicit and conservative in system-critical code.
* Consider security implications of every change.
* Avoid introducing deployment-specific assumptions.

---

## Pull request process

1. Fork the repository and create a feature branch.
2. Make small, focused commits with clear commit messages.
3. Ensure the code builds and existing tests pass.
4. Open a pull request using the provided template.
5. Ensure the CLA check passes.

Each pull request should:

* address a single issue or feature,
* include a clear description of the changes,
* reference related issues if applicable.

---

## Licensing

By contributing to this repository, you agree that your contributions
will be licensed under the same license as the project.

Any third-party code must use a compatible license and be clearly documented.

