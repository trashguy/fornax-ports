#!/bin/sh
#
# mk-ca-bundle.sh — regenerate ca-certificates from Mozilla's certdata.txt
#
# Downloads curl's pre-built PEM bundle (derived from Mozilla NSS certdata.txt)
# and prints the SHA-256 for updating the FAYBUILD.
#
# Usage: ./mk-ca-bundle.sh
#
set -e

URL="https://curl.se/ca/cacert.pem"
OUT="cacert.pem"

echo "Downloading $URL ..."
curl -sL "$URL" -o "$OUT"

SIZE=$(wc -c < "$OUT")
DATE=$(head -5 "$OUT" | grep "as of:" | sed 's/.*as of: //')
SHA=$(sha256sum "$OUT" | cut -d' ' -f1)

echo "File:    $OUT ($SIZE bytes)"
echo "Date:    $DATE"
echo "SHA-256: $SHA"
echo ""
echo "Update FAYBUILD:"
echo "  pkgver  → $(echo "$DATE" | awk '{print $5"."$2"."$3}' | sed 's/Jan/01/;s/Feb/02/;s/Mar/03/;s/Apr/04/;s/May/05/;s/Jun/06/;s/Jul/07/;s/Aug/08/;s/Sep/09/;s/Oct/10/;s/Nov/11/;s/Dec/12/')"
echo "  sha256  → $SHA"
