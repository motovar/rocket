#!/usr/bin/env bash
# Makes a signing key for your builds and prints the four values to add as GitHub repository secrets
# (Settings > Secrets and variables > Actions > New repository secret). Run once; keep rocket-release.jks safe and OUT of git.
set -euo pipefail
ALIAS=rocket
PASS="$(openssl rand -hex 16)"
keytool -genkeypair -keystore rocket-release.jks -storepass "$PASS" -keypass "$PASS" -alias "$ALIAS" \
  -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Rocket"
echo
echo "Add these four secrets to the GitHub repository:"
echo "  ANDROID_KEYSTORE_PASSWORD = $PASS"
echo "  ANDROID_KEY_ALIAS         = $ALIAS"
echo "  ANDROID_KEY_PASSWORD      = $PASS"
echo "  ANDROID_KEYSTORE_BASE64   = (the single line below)"
base64 -w0 rocket-release.jks 2>/dev/null || base64 rocket-release.jks | tr -d '\n'
echo
