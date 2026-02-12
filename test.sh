#!/bin/bash

set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

SNAPSHOT_DB="$PROJECT_DIR/db/sanitized.db"

TEST_ADMIN_USERNAME="Account1"
TEST_ADMIN_PASSWORD="123456"
TEST_USER_USERNAME="Account3"
TEST_USER_PASSWORD="123456"

echo "[1/4] build (Debug)"
"$PROJECT_DIR/build.sh" --debug >/dev/null

echo "[2/4] db tests (ctest)"
cd "$BUILD_DIR"
ctest --output-on-failure

TMP_DIR="$(mktemp -d)"
cleanup() {
  if [ -n "${SERVER_PID:-}" ]; then
    kill "$SERVER_PID" >/dev/null 2>&1 || true
    wait "$SERVER_PID" >/dev/null 2>&1 || true
  fi
  rm -rf "$TMP_DIR"
}
trap cleanup EXIT

PORT="${BLOG_TEST_PORT:-18080}"

python3 - "$PORT" <<'PY'
import socket, sys
port = int(sys.argv[1])
s = socket.socket()
s.settimeout(0.2)
try:
    s.connect(("127.0.0.1", port))
except OSError:
    sys.exit(0)
else:
    sys.stderr.write(f"port {port} is already in use\n")
    sys.exit(2)
finally:
    s.close()
PY

DB_PATH="$TMP_DIR/blog_test.db"
if [ ! -f "$SNAPSHOT_DB" ]; then
  echo "snapshot db not found: $SNAPSHOT_DB" >&2
  echo "please create it first: $SNAPSHOT_DB" >&2
  exit 2
fi
cp "$SNAPSHOT_DB" "$DB_PATH"

echo "[3/4] start server"
cd "$BUILD_DIR"
BLOG_DB_PATH="$DB_PATH" BLOG_PORT="$PORT" ./bin/blog_server >"$TMP_DIR/server.log" 2>&1 &
SERVER_PID="$!"

READY=0
for _ in $(seq 1 80); do
  CODE="$(curl -s -o /dev/null -w "%{http_code}" "http://127.0.0.1:$PORT/" || true)"
  if [ "$CODE" != "000" ]; then
    READY=1
    break
  fi
  sleep 0.1
done

if [ "$READY" != "1" ]; then
  echo "server did not start" >&2
  tail -n 200 "$TMP_DIR/server.log" >&2 || true
  exit 1
fi

echo "[4/4] http tests"
BLOG_TEST_ADMIN_USERNAME="$TEST_ADMIN_USERNAME" \
BLOG_TEST_ADMIN_PASSWORD="$TEST_ADMIN_PASSWORD" \
BLOG_TEST_USER_USERNAME="$TEST_USER_USERNAME" \
BLOG_TEST_USER_PASSWORD="$TEST_USER_PASSWORD" \
python3 "$PROJECT_DIR/test/test-e2e.py" --base-url "http://127.0.0.1:$PORT"

echo "ok"
