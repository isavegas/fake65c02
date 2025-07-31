#!/usr/bin/env sh

SUPERVISION="$1"
shift
TMP_DIR="$1"
shift
OUTPUT="$1"
shift
OBJECTS="$@"

TMP_FILE="$TMP_DIR/$(basename "$OUTPUT")"

mkdir -p "$TMP_DIR" || exit 1
cp "$SUPERVISION" "$TMP_FILE" || exit 1
"$AR" a "$TMP_FILE" $OBJECTS || exit 1
mv "$TMP_FILE" "$OUTPUT" || exit 1
