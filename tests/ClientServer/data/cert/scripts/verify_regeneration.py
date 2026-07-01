#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

"""Compare PKI test DER metadata before/after regeneration (not byte-identical)."""

import argparse
import shutil
import subprocess
import sys
from datetime import timezone
from pathlib import Path

from cryptography import x509

MIGRATED_TESTS = ("expired_in_pki_test", "revocation_pki_test", "signature_pki_test")


# Compatibility shim for cryptography < 42.0 (naive UTC attrs). Drop when >= 42.0
# is the minimum and call *_utc.isoformat() directly.
def dt_iso_utc(obj, attr_base):
    """Return an ISO8601 UTC string from x509 *_utc or legacy naive UTC attrs."""
    dt = getattr(obj, f"{attr_base}_utc", None)
    if dt is None:
        dt = getattr(obj, attr_base).replace(tzinfo=timezone.utc)
    return dt.isoformat()


def cert_summary(path):
    cert = x509.load_der_x509_certificate(path.read_bytes())
    return {
        "subject": cert.subject.rfc4514_string(),
        "issuer": cert.issuer.rfc4514_string(),
        "serial": cert.serial_number,
        "not_before": dt_iso_utc(cert, "not_valid_before"),
        "not_after": dt_iso_utc(cert, "not_valid_after"),
    }


def crl_summary(path):
    crl = x509.load_der_x509_crl(path.read_bytes())
    revoked = sorted(entry.serial_number for entry in crl)
    return {
        "issuer": crl.issuer.rfc4514_string(),
        "last_update": dt_iso_utc(crl, "last_update"),
        "next_update": dt_iso_utc(crl, "next_update"),
        "revoked": revoked,
    }


def summarize_test_dir(test_dir):
    summaries = {}
    for path in sorted(test_dir.rglob("*.der")):
        rel = path.relative_to(test_dir)
        if "crl" in path.parts:
            summaries[str(rel)] = crl_summary(path)
        else:
            summaries[str(rel)] = cert_summary(path)
    return summaries


def verify_test_dir(cert_root, test_name):
    test_dir = cert_root / test_name
    backup_dir = cert_root / f".{test_name}_backup"

    if not (test_dir / "generate_certs.py").is_file():
        print(f"Skipping {test_name}: generate_certs.py not found")
        return True

    if backup_dir.exists():
        shutil.rmtree(backup_dir)
    shutil.copytree(test_dir, backup_dir, ignore=shutil.ignore_patterns("generate_certs.py", "README"))

    before = summarize_test_dir(test_dir)

    result = subprocess.run(
        [sys.executable, "generate_certs.py"],
        cwd=test_dir,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr, file=sys.stderr)
        shutil.rmtree(backup_dir)
        return False

    after = summarize_test_dir(test_dir)

    for path in backup_dir.rglob("*.der"):
        rel = path.relative_to(backup_dir)
        shutil.copy2(path, test_dir / rel)
    shutil.rmtree(backup_dir)

    if before != after:
        print(f"Metadata mismatch for {test_name}:")
        for key in sorted(set(before) | set(after)):
            if before.get(key) != after.get(key):
                print(f"  {key}:")
                print(f"    before: {before.get(key)}")
                print(f"    after:  {after.get(key)}")
        return False

    print(f"Regenerated {test_name} DER metadata matches committed files.")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "tests",
        nargs="*",
        default=MIGRATED_TESTS,
        help=f"Test subdirectories to verify (default: {' '.join(MIGRATED_TESTS)})",
    )
    args = parser.parse_args()

    cert_root = Path(__file__).resolve().parent.parent
    ok = True
    for test_name in args.tests:
        if not verify_test_dir(cert_root, test_name):
            ok = False
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
