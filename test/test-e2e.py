import argparse
import base64
import hashlib
import hmac
import json
import os
import random
import string
import sys
import time
import urllib.error
import urllib.request
from typing import Optional


def _rand_suffix(n: int = 10) -> str:
    alphabet = string.ascii_lowercase + string.digits
    return "".join(random.choice(alphabet) for _ in range(n))


def _request_json(method: str, url: str, token: Optional[str] = None, body: Optional[dict] = None):
    data = None
    headers = {"Accept": "application/json"}
    if body is not None:
        data = json.dumps(body).encode("utf-8")
        headers["Content-Type"] = "application/json"
    if token:
        headers["Authorization"] = f"Bearer {token}"
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            raw = resp.read().decode("utf-8")
            try:
                return resp.status, json.loads(raw)
            except Exception:
                return resp.status, raw
    except urllib.error.HTTPError as e:
        raw = e.read().decode("utf-8")
        try:
            return e.code, json.loads(raw)
        except Exception:
            return e.code, raw


def _assert(cond: bool, msg: str):
    if not cond:
        raise AssertionError(msg)


def _jwt_generate(user_id: int, username: str, role: str) -> str:
    secret = os.environ.get("BLOG_JWT_SECRET", "blog_secret_key_2026")
    header = {"alg": "HS256", "typ": "JWT"}
    now = int(time.time())
    payload = {"sub": user_id, "username": username, "role": role, "iat": now, "exp": now + 3600}

    header_b64 = base64.b64encode(json.dumps(header, separators=(",", ":")).encode("utf-8")).decode("ascii")
    payload_b64 = base64.b64encode(json.dumps(payload, separators=(",", ":")).encode("utf-8")).decode("ascii")
    message = f"{header_b64}.{payload_b64}".encode("ascii")

    digest = hmac.new(secret.encode("utf-8"), message, hashlib.sha256).digest()
    signature_b64 = base64.b64encode(digest).decode("ascii")
    return f"{header_b64}.{payload_b64}.{signature_b64}"


def _extract(data: dict, path: str):
    cur = data
    for part in path.split("."):
        if not isinstance(cur, dict) or part not in cur:
            return None
        cur = cur[part]
    return cur


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base-url", required=True)
    args = parser.parse_args()

    base_url = args.base_url.rstrip("/")

    st, _ = _request_json("GET", f"{base_url}/api/posts")
    _assert(st == 401, f"expected 401 for unauthenticated GET /api/posts, got {st}")

    user_username = os.environ.get("BLOG_TEST_USER_USERNAME")
    user_password = os.environ.get("BLOG_TEST_USER_PASSWORD")
    if user_username and user_password:
        st, data = _request_json(
            "POST",
            f"{base_url}/api/auth/login",
            body={"username": user_username, "password": user_password},
        )
        _assert(st == 200, f"user login expected 200, got {st}, body={data}")
        user_token = _extract(data, "data.token")
        user_id = _extract(data, "data.user_id")
        user_role = _extract(data, "data.role")
        _assert(user_token, f"user login missing token, body={data}")
        _assert(isinstance(user_id, int) and user_id > 0, f"user login missing user_id, body={data}")
        _assert(user_role == "user", f"user role expected 'user', got {user_role}")
    else:
        user_username = f"user_{_rand_suffix()}"
        user_password = "password123"
        st, data = _request_json(
            "POST",
            f"{base_url}/api/auth/register",
            body={"username": user_username, "password": user_password},
        )
        _assert(st == 201, f"user register expected 201, got {st}, body={data}")
        user_token = _extract(data, "data.token")
        user_id = _extract(data, "data.user_id")
        _assert(user_token, f"user register missing token, body={data}")
        _assert(isinstance(user_id, int) and user_id > 0, f"user register missing user_id, body={data}")

    user2_username = f"user2_{_rand_suffix()}"
    user2_password = "password123"
    st, data = _request_json(
        "POST",
        f"{base_url}/api/auth/register",
        body={"username": user2_username, "password": user2_password},
    )
    _assert(st == 201, f"user2 register expected 201, got {st}, body={data}")
    user2_token = _extract(data, "data.token")
    _assert(user2_token, f"user2 register missing token, body={data}")

    admin_username = os.environ.get("BLOG_TEST_ADMIN_USERNAME")
    admin_password = os.environ.get("BLOG_TEST_ADMIN_PASSWORD")
    if admin_username and admin_password:
        st, data = _request_json(
            "POST",
            f"{base_url}/api/auth/login",
            body={"username": admin_username, "password": admin_password},
        )
        _assert(st == 200, f"admin login expected 200, got {st}, body={data}")
        admin_token = _extract(data, "data.token")
        admin_role = _extract(data, "data.role")
        _assert(admin_token, f"admin login missing token, body={data}")
        _assert(admin_role == "admin", f"admin role expected 'admin', got {admin_role}")
    else:
        admin_token = None

    st, data = _request_json(
        "POST",
        f"{base_url}/api/posts",
        token=user_token,
        body={"title": "t_user1", "content": "c_user1"},
    )
    _assert(st == 201, f"create post1 expected 201, got {st}, body={data}")
    post1_id = _extract(data, "data.id")
    _assert(isinstance(post1_id, int) and post1_id > 0, f"create post1 missing id, body={data}")

    st, data = _request_json(
        "POST",
        f"{base_url}/api/posts",
        token=user2_token,
        body={"title": "t_user2", "content": "c_user2"},
    )
    _assert(st == 201, f"create post2 expected 201, got {st}, body={data}")
    post2_id = _extract(data, "data.id")
    _assert(isinstance(post2_id, int) and post2_id > 0, f"create post2 missing id, body={data}")

    st, data = _request_json(
        "PUT",
        f"{base_url}/api/posts/{post1_id}",
        token=user_token,
        body={"title": "t_user1_updated", "content": "c_user1_updated"},
    )
    _assert(st == 200, f"user1 update own post expected 200, got {st}, body={data}")
    _assert(_extract(data, "success") is True, f"user1 update own post expected success=true, body={data}")

    st, data = _request_json("GET", f"{base_url}/api/posts/{post1_id}", token=user_token)
    _assert(st == 200, f"user1 get own updated post expected 200, got {st}, body={data}")
    _assert(_extract(data, "data.title") == "t_user1_updated", f"post1 title not updated, body={data}")
    _assert(_extract(data, "data.content") == "c_user1_updated", f"post1 content not updated, body={data}")

    st, _ = _request_json(
        "PUT",
        f"{base_url}/api/posts/{post1_id}",
        token=user2_token,
        body={"title": "hijack", "content": "hijack"},
    )
    _assert(st == 403, f"user2 update user1 post expected 403, got {st}")

    st, _ = _request_json(
        "PUT",
        f"{base_url}/api/posts/{post1_id}",
        token=admin_token,
        body={"title": "admin_edit", "content": "admin_edit"},
    ) if admin_token else (None, None)
    if admin_token:
        _assert(st == 403, f"admin update other user's post expected 403, got {st}")

    st, _ = _request_json(
        "PUT",
        f"{base_url}/api/posts/99999999",
        token=user_token,
        body={"title": "x", "content": "y"},
    )
    _assert(st == 404, f"update non-existent post expected 404, got {st}")

    st, data = _request_json("GET", f"{base_url}/api/posts", token=user_token)
    _assert(st == 200, f"user1 list posts expected 200, got {st}, body={data}")
    posts = _extract(data, "data")
    _assert(isinstance(posts, list), f"user1 list posts expected list, body={data}")
    ids = {p.get("id") for p in posts if isinstance(p, dict)}
    _assert(post1_id in ids, f"user1 should see own post {post1_id}")
    _assert(post2_id not in ids, f"user1 should not see other post {post2_id}")

    if admin_token:
        st, data = _request_json("GET", f"{base_url}/api/posts", token=admin_token)
        _assert(st == 200, f"admin list posts expected 200, got {st}, body={data}")
        posts = _extract(data, "data")
        _assert(isinstance(posts, list), f"admin list posts expected list, body={data}")
        ids = {p.get("id") for p in posts if isinstance(p, dict)}
        _assert(post1_id in ids and post2_id in ids, "admin should see all posts")

    st, _ = _request_json("GET", f"{base_url}/api/posts/{post1_id}", token=user2_token)
    _assert(st == 403, f"user2 get user1 post expected 403, got {st}")

    if admin_token:
        st, _ = _request_json("GET", f"{base_url}/api/posts/{post1_id}", token=admin_token)
        _assert(st == 200, f"admin get post1 expected 200, got {st}")

    st, _ = _request_json("DELETE", f"{base_url}/api/posts/{post1_id}", token=user2_token)
    _assert(st == 403, f"user2 delete post1 expected 403, got {st}")

    if admin_token:
        st, data = _request_json("DELETE", f"{base_url}/api/posts/{post1_id}", token=admin_token)
        _assert(st == 200, f"admin delete post1 expected 200, got {st}, body={data}")

        st, data = _request_json("GET", f"{base_url}/api/posts/{post1_id}", token=admin_token)
        _assert(st == 200, f"admin get post1 after delete expected 200, got {st}, body={data}")
        deleted_by_admin = _extract(data, "data.deleted_by_admin")
        deleted_at = _extract(data, "data.deleted_at")
        _assert(deleted_by_admin == 1, f"expected deleted_by_admin=1, got {deleted_by_admin}, body={data}")
        _assert(isinstance(deleted_at, int) and deleted_at > 0, f"expected deleted_at>0, got {deleted_at}, body={data}")

        st, _ = _request_json(
            "PUT",
            f"{base_url}/api/posts/{post1_id}",
            token=user_token,
            body={"title": "should_fail", "content": "should_fail"},
        )
        _assert(st == 403, f"user1 update admin-deleted post expected 403, got {st}")

    st, data = _request_json("DELETE", f"{base_url}/api/posts/{post2_id}", token=user2_token)
    _assert(st == 200, f"user2 delete own post2 expected 200, got {st}, body={data}")

    if admin_token:
        st, _ = _request_json("GET", f"{base_url}/api/posts/{post2_id}", token=admin_token)
        _assert(st == 404, f"admin get deleted post2 expected 404, got {st}")

    if admin_token:
        st, data = _request_json("GET", f"{base_url}/api/posts", token=user_token)
        _assert(st == 200, f"user1 list posts after delete expected 200, got {st}, body={data}")
        posts = _extract(data, "data")
        post1 = None
        for p in posts:
            if isinstance(p, dict) and p.get("id") == post1_id:
                post1 = p
                break
        _assert(post1 is not None, "user1 should still see admin-deleted post in own list")
        _assert(post1.get("deleted_by_admin") == 1, "user1 list should show deleted_by_admin=1 for post1")
        _assert(post1.get("is_author") is False, "user1 list should mark is_author=false for admin-deleted post1")

    st, data = _request_json(
        "POST",
        f"{base_url}/api/auth/delete",
        token=user_token,
        body={"password": user_password},
    )
    _assert(st == 200, f"user delete account expected 200, got {st}, body={data}")
    _assert(_extract(data, "success") is True, f"user delete account expected success=true, body={data}")

    st, _ = _request_json("GET", f"{base_url}/api/posts", token=user_token)
    _assert(st == 401, f"expected 401 for deleted-account token GET /api/posts, got {st}")

    st, _ = _request_json("POST", f"{base_url}/api/auth/verify", body={"token": user_token})
    _assert(st == 401, f"expected 401 for deleted-account token POST /api/auth/verify, got {st}")

    if admin_token:
        st, data = _request_json("GET", f"{base_url}/api/posts", token=admin_token)
        _assert(st == 200, f"admin list posts after account deletion expected 200, got {st}, body={data}")
        posts = _extract(data, "data")
        _assert(isinstance(posts, list), f"admin list posts expected list, body={data}")
        ids = {p.get("id") for p in posts if isinstance(p, dict)}
        _assert(post1_id not in ids, "admin should not see deleted user's post1 after account deletion")

    st, data = _request_json(
        "POST",
        f"{base_url}/api/auth/register",
        body={"username": user_username, "password": "password123"},
    )
    _assert(st == 201, f"re-register after account deletion expected 201, got {st}, body={data}")
    new_user_id = _extract(data, "data.user_id")
    _assert(isinstance(new_user_id, int) and new_user_id > 0, f"re-register missing user_id, body={data}")
    _assert(new_user_id != user_id, f"re-register should get new user_id, old={user_id}, new={new_user_id}")

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as e:
        sys.stderr.write(str(e) + "\n")
        raise SystemExit(1)
