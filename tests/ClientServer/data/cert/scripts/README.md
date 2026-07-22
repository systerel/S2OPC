# PKI test certificate generation library

Shared Python helpers for regenerating DER files under `tests/ClientServer/data/cert/`.

Each test subdirectory keeps a thin `generate_certs.py` with scenario-specific data
(CA names, serial numbers, validity dates, output filenames). Common X.509 building
logic lives in `pki_gen/`.

## Requirements

- Python 3
- [`cryptography`](https://pypi.org/project/cryptography/) (`pip install cryptography`)

## Usage

From a test subdirectory:

```bash
cd tests/ClientServer/data/cert/revocation_pki_test
python3 generate_certs.py
```

Private keys are generated at runtime and must not be committed. DER files are
committed so unit tests do not depend on generation at build time.

## Layout

```
scripts/
  pki_gen/
    common.py       # systerel_name, write_der, date constants
    extensions.py   # CA / leaf / self-signed X.509 extensions
    builders.py     # build_ca, build_leaf, build_crl, build_selfsigned
    generator.py    # PkiStoreGenerator, CaSpec, CrlSpec, LeafSpec
    signature.py    # corrupt_der_signature (internal)
  verify_regeneration.py
```

## API highlights for signature_pki_test

- Root CA certificates are built with `build_ca()` (same X.509 profile as other CAs).
- `PkiStoreGenerator.write_*_corrupted()` writes corrupted trusted certs, CRLs, or
  leaf certificates (used for bad-signature test cases, including leaf case 5).

## Regenerating migrated stores

```bash
cd tests/ClientServer/data/cert
for d in expired_in_pki_test revocation_pki_test signature_pki_test cycle_pki_test; do
  (cd "$d" && python3 generate_certs.py)
done
```

After regeneration, verify committed DER files are unchanged (`git diff`), or run:

```bash
python3 scripts/verify_regeneration.py
python3 scripts/verify_regeneration.py signature_pki_test
```

This compares X.509 metadata (subject, serial, validity, CRL revocations) between
committed DER files and a fresh regeneration. Byte-identical output is not expected:
private RSA keys are generated at runtime and are not committed.
