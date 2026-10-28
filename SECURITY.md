# Security Policy

This document describes how to report a security vulnerability in S2OPC and what to expect
from the S2OPC team in return. It follows the structure recommended by the
[GitLab Public Sector SECURITY.md standard](https://gitlab-com.gitlab.io/public-sector/reference/standards/security-md/).

## Supported Versions

Security fixes are only applied to the `master` branch. Users are therefore advised to always
base their work on the latest release or the most recent commit of `master`. Older releases do
not receive backported security fixes.

## Reporting a Vulnerability

If you believe you have found a cybersecurity vulnerability in S2OPC, please report it
responsibly using one of the following routes:

1. **Primary (preferred):** open a confidential issue on the
   [project's issue tracker](https://gitlab.com/systerel/S2OPC/-/issues/new?issue%5Bconfidential%5D=true).
   Confidential issues are only visible to the S2OPC maintainers.
2. **Backup:** if you are unable to create a confidential issue (e.g. no GitLab account, or the
   form is unavailable), send a notification email to `support.cyber@systerel.fr`.

Please do not disclose the vulnerability publicly (issue tracker, mailing list, social media,
etc.) until the S2OPC team has had the opportunity to analyze and address it.

When reporting, please include as much of the following as possible:
- Affected version / commit hash and build configuration
- A description of the vulnerability and its potential impact
- Steps to reproduce, or a proof-of-concept

## Severity Scale

Reported vulnerabilities are assessed using [CVSS v3.1](https://www.first.org/cvss/calculator/3.1)
scoring

## Scope

**In scope:**
- The S2OPC Client/Server and PubSub source code (`src/`), including the OPC UA communication
  stack, B model, generated code, and the cryptographic library adaptation.

**Out of scope:**
- Vulnerabilities in third-party dependencies that are unmodified by S2OPC (e.g. mbedtls,
  CycloneCRYPTO, expat, Paho, libcheck) — please report these directly upstream.
- Vulnerabilities in GitLab itself, or in the OPC UA specification.
- Issues requiring an unsupported or unofficial build configuration not documented in
  [README.md](README.md).

## After a Fix is Released

- The confidential issue is made public once a fix is released.
- The associated changelog/release notes reference the fix and any assigned CVE.
- The reporter is credited (by their preferred name or handle) unless they request anonymity.

## Commercial Support

For S2OPC commercial support (non security-related), please contact `s2opc@systerel.fr`.
