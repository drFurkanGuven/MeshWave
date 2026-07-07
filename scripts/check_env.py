#!/usr/bin/env python3
"""
LoRaLink pre-build environment check.
Validates that the active PlatformIO environment is cardputer or gateway.
"""
Import("env")

env_name = env.get("PIOENV", "unknown")
allowed = {"cardputer", "gateway"}

if env_name not in allowed:
    print(f"[LoRaLink] WARNING: unknown env '{env_name}', expected {allowed}")

role = "terminal" if env_name == "cardputer" else "gateway"
print(f"[LoRaLink] Building for: {env_name} ({role})")
