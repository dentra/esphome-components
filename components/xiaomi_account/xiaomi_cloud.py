# Xiaomi Cloud client with QR code login.
#
# Based on https://github.com/AlexxIT/XiaomiGateway3 (core/xiaomi_cloud.py)
# Copyright (c) 2020 AlexxIT
#
# QR code login based on https://github.com/PiotrMachowski/Xiaomi-cloud-tokens-extractor
# Copyright (c) 2020 Piotr Machowski
#
# Both are distributed under the MIT License:
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import base64
import hashlib
import io
import json
import logging
import os
import random
import string
import time
from dataclasses import dataclass

import requests
from cryptography.hazmat.primitives import ciphers

try:
    from cryptography.hazmat.decrepit.ciphers import algorithms
except ImportError:
    # cryptography < 43
    from cryptography.hazmat.primitives.ciphers import algorithms

_LOGGER = logging.getLogger(__name__)

SID = "xiaomiio"
REQUEST_TIMEOUT = 30
LONG_POLLING_TIMEOUT = 10
USER_AGENT = "Android-7.1.1-1.0.0-ONEPLUS A3010-136-{} APP/xiaomi.smarthome APPV/62830"


class XiaomiCloudError(Exception):
    pass


class XiaomiAuthError(XiaomiCloudError):
    pass


@dataclass
class QrLogin:
    # URL of the QR code image
    image_url: str
    # URL encoded in the QR code, can be opened in a browser
    login_url: str
    long_polling_url: str
    timeout: int


class XiaomiCloud:
    def __init__(self):
        self._session = requests.Session()
        self._session.headers["User-Agent"] = USER_AGENT.format(
            get_random_string(16)
        )
        self._cookies: dict | None = None
        self._ssecurity: bytes | None = None

    def login_token(self, token: str) -> str:
        """Login with token from the previous login. Returns a new token."""
        user_id, pass_token = token.split(":", 1)
        r = self._session.get(
            "https://account.xiaomi.com/pass/serviceLogin",
            cookies={"userId": user_id, "passToken": pass_token},
            params={"_json": "true", "sid": SID},
            timeout=REQUEST_TIMEOUT,
        )
        res = parse_auth_response(r.content)
        if res.get("code") != 0 or not res.get("location"):
            raise XiaomiAuthError(f"Token login failed: {res.get('desc', res)}")
        res.setdefault("userId", user_id)
        res.setdefault("passToken", pass_token)
        return self._get_credentials(res)

    def login_qr_start(self) -> QrLogin:
        r = self._session.get(
            "https://account.xiaomi.com/longPolling/loginUrl",
            params={
                "_qrsize": "240",
                "qs": f"%3Fsid%3D{SID}%26_json%3Dtrue",
                "callback": "https://sts.api.io.mi.com/sts",
                "_hasLogo": "false",
                "sid": SID,
                "serviceParam": "",
                "_locale": "en_GB",
                "_dc": str(int(time.time() * 1000)),
            },
            timeout=REQUEST_TIMEOUT,
        )
        res = parse_auth_response(r.content)
        if res.get("code") != 0 or "qr" not in res:
            raise XiaomiCloudError(f"Unable to get QR code: {res.get('desc', res)}")
        return QrLogin(
            image_url=res["qr"],
            login_url=res["loginUrl"],
            long_polling_url=res["lp"],
            timeout=int(res.get("timeout", 300)),
        )

    def get_qr_image(self, qr: QrLogin) -> bytes:
        r = self._session.get(qr.image_url, timeout=REQUEST_TIMEOUT)
        r.raise_for_status()
        return r.content

    def login_qr_wait(self, qr: QrLogin) -> str:
        """Wait until QR code is scanned. Returns token for the next login."""
        deadline = time.monotonic() + qr.timeout
        while time.monotonic() < deadline:
            try:
                r = self._session.get(
                    qr.long_polling_url, timeout=LONG_POLLING_TIMEOUT
                )
            except requests.exceptions.Timeout:
                continue
            if r.status_code == 200:
                return self._get_credentials(parse_auth_response(r.content))
            _LOGGER.debug("Long polling status %s, retrying", r.status_code)
            time.sleep(1)
        raise XiaomiCloudError("QR code login timed out")

    def _get_credentials(self, data: dict) -> str:
        r = self._session.get(data["location"], timeout=REQUEST_TIMEOUT)
        if r.content != b"ok":
            raise XiaomiCloudError(f"Unable to get service token: {r.text[:100]}")

        self._cookies = r.cookies.get_dict()
        if "serviceToken" not in self._cookies:
            raise XiaomiCloudError("Service token not found")
        self._cookies.setdefault("userId", str(data["userId"]))
        self._ssecurity = base64.b64decode(data["ssecurity"])

        return f"{data['userId']}:{data['passToken']}"

    def request(self, server: str, path: str, params: dict) -> dict:
        form: dict[str, str] = {"data": json.dumps(params, separators=(",", ":"))}

        nonce = gen_nonce()
        signed_nonce = gen_signed_nonce(self._ssecurity, nonce)

        # 1. gen hash for data param
        form["rc4_hash__"] = gen_signature_base64(path, form, signed_nonce)

        # 2. encrypt data and hash params
        for k, v in form.items():
            form[k] = base64.b64encode(crypt(signed_nonce, v.encode())).decode()

        # 3. add signature for encrypted data and hash params
        form["signature"] = gen_signature_base64(path, form, signed_nonce)

        # 4. add nonce
        form["_nonce"] = base64.b64encode(nonce).decode()

        r = self._session.post(
            get_api_url(server) + path,
            cookies=self._cookies,
            data=form,
            timeout=REQUEST_TIMEOUT,
        )
        r.raise_for_status()

        res = json.loads(crypt(signed_nonce, base64.b64decode(r.content)))
        if res.get("code") != 0:
            raise XiaomiCloudError(f"Request {path} failed: {res}")
        return res["result"]

    def get_devices(self, server: str) -> list[dict]:
        payload = {
            "limit": 200,
            "getVirtualModel": True,
            "getHuamiDevices": 1,
            "get_split_device": False,
            "support_smart_home": True,
        }
        devices = []
        seen = set()
        while True:
            res = self.request(server, "/v2/home/device_list_page", payload)
            devices.extend(res.get("list") or [])
            next_start_did = res.get("next_start_did")
            if not res.get("has_more") or not next_start_did or next_start_did in seen:
                return devices
            seen.add(next_start_did)
            payload["start_did"] = next_start_did

    def get_bindkey(self, server: str, did: str) -> str | None:
        res = self.request(
            server, "/v2/device/blt_get_beaconkey", {"did": did, "pdid": 1}
        )
        return res.get("beaconkey") if res else None


def get_api_url(server: str) -> str:
    return f"https://{'' if server == 'cn' else server + '.'}api.io.mi.com/app"


def parse_auth_response(body: bytes) -> dict:
    if not body.startswith(b"&&&START&&&"):
        raise XiaomiCloudError(f"Unexpected response: {body[:100]}")
    return json.loads(body[11:])


def get_random_string(length: int) -> str:
    seq = string.ascii_uppercase + string.digits
    return "".join(random.choice(seq) for _ in range(length))


def gen_nonce() -> bytes:
    return os.urandom(8) + int(time.time() / 60).to_bytes(4, "big")


def gen_signed_nonce(ssecurity: bytes, nonce: bytes) -> bytes:
    return hashlib.sha256(ssecurity + nonce).digest()


def gen_signature_base64(path: str, data: dict, signed_nonce: bytes) -> str:
    params = ["POST", path]
    for k, v in data.items():
        params.append(f"{k}={v}")
    # use b64 signed nonce
    params.append(base64.b64encode(signed_nonce).decode())
    signature = hashlib.sha1("&".join(params).encode()).digest()
    return base64.b64encode(signature).decode()


def crypt(key: bytes, data: bytes) -> bytes:
    cipher = ciphers.Cipher(algorithms.ARC4(key), None)
    encryptor = cipher.encryptor()
    encryptor.update(bytes(1024))
    return encryptor.update(data)


def render_qr(image: bytes) -> str | None:
    """Render QR code image as text with half block characters.
    Returns None when image could not be recognized as QR code."""
    try:
        from PIL import Image
    except ImportError:
        return None

    img = Image.open(io.BytesIO(image)).convert("RGBA")
    # transparent pixels must be treated as white
    background = Image.new("RGBA", img.size, "white")
    img = Image.alpha_composite(background, img).convert("L")
    pixels = img.load()

    def is_dark(x: float, y: float) -> bool:
        return pixels[int(x), int(y)] < 128

    bbox = img.point(lambda p: 255 if p < 128 else 0).getbbox()
    if not bbox:
        return None
    left, top, right, bottom = bbox

    # top left finder pattern is 7 modules wide
    run = 0
    while left + run < right and is_dark(left + run, top):
        run += 1
    module = run / 7
    count = round((right - left) / module) if module >= 1 else 0
    if count < 21 or (count - 21) % 4 != 0:
        return None

    quiet = 2
    size = count + quiet * 2
    grid = [[False] * size for _ in range(size + 1)]
    for row in range(count):
        for col in range(count):
            grid[row + quiet][col + quiet] = is_dark(
                left + (col + 0.5) * module, top + (row + 0.5) * module
            )

    # black foreground on white background, independent of terminal theme
    lines = []
    for row in range(0, size, 2):
        chars = "".join(
            " ▄▀█"[grid[row][col] * 2 + grid[row + 1][col]] for col in range(size)
        )
        lines.append(f"\033[30;47m{chars}\033[0m")
    return "\n".join(lines)
