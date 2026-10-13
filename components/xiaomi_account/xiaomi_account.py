import io
import json
import logging
import os
import time
from pathlib import Path

from .xiaomi_cloud import (
    XiaomiAuthError,
    XiaomiCloud,
    XiaomiCloudError,
    render_qr,
)

_LOGGER = logging.getLogger(__name__)


class XiaomiAccount:
    def __init__(
        self,
        servers: list[str],
        session_file: Path,
        storage_path: str = ".",
        update_interval: float = 60 * 60,
        render_qr: bool = True,
    ):
        self._servers = servers
        self._render_qr = render_qr
        self._session_file = session_file
        self._storage = Path(storage_path) / ".xiaomi-devices.json"
        self._update_interval = update_interval
        self._devices: list[dict] | None = None

    def _read_session(self) -> str | None:
        try:
            with io.open(self._session_file, encoding="utf8") as f:
                return json.load(f).get("token")
        except (OSError, ValueError):
            return None

    def _write_session(self, token: str):
        try:
            self._session_file.parent.mkdir(parents=True, exist_ok=True)
            tmp = self._session_file.with_suffix(".tmp")
            # token gives full access to the account, so make it readable by owner only
            fd = os.open(tmp, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
            with io.open(fd, "w", encoding="utf8") as f:
                json.dump({"token": token}, f)
            os.chmod(tmp, 0o600)
            os.replace(tmp, self._session_file)
        except OSError as err:
            _LOGGER.error(
                "Unable to store Xiaomi account session to %s: %s",
                self._session_file,
                err,
            )

    def _login(self) -> XiaomiCloud | None:
        cloud = XiaomiCloud()

        token = self._read_session()
        if token:
            try:
                token = cloud.login_token(token)
            except XiaomiAuthError as err:
                _LOGGER.warning("Stored Xiaomi account session is invalid: %s", err)
                token = None
            except Exception as err:
                # do not ask for QR code login when the session may still be valid
                _LOGGER.error("Unable to login into Xiaomi account: %s", err)
                return None
            if token:
                self._write_session(token)
                return cloud

        try:
            qr = cloud.login_qr_start()
            text = None
            if self._render_qr:
                try:
                    text = render_qr(cloud.get_qr_image(qr))
                except Exception as err:
                    _LOGGER.debug("Unable to render QR code: %s", err)
            _LOGGER.warning(
                "Xiaomi account login required.\n"
                "%s with the Mi Home app or open the login link "
                "in a browser where you are logged in to https://account.xiaomi.com\n"
                "%s"
                "QR code image: %s\n"
                "Login link: %s\n"
                "Waiting %s seconds for confirmation...",
                "Scan the QR code" if text else "Open the QR code image link and scan it",
                f"{text}\n" if text else "",
                qr.image_url,
                qr.login_url,
                qr.timeout,
            )
            token = cloud.login_qr_wait(qr)
        except Exception as err:
            _LOGGER.error("Unable to login into Xiaomi account: %s", err)
            return None

        _LOGGER.info("Logged in to Xiaomi account")
        self._write_session(token)
        return cloud

    def _fetch_devices(self, cloud: XiaomiCloud, server: str) -> list[dict]:
        _LOGGER.debug("Processing server %s", server)
        devices = []
        for device in cloud.get_devices(server):
            did = device.get("did") or ""
            if "blt" not in did:
                continue
            _LOGGER.debug('Processing device "%s"', device.get("name"))
            try:
                beaconkey = cloud.get_bindkey(server, did)
            except XiaomiCloudError as err:
                _LOGGER.warning(
                    'Unable to get bindkey for device "%s": %s', device.get("name"), err
                )
                continue
            if not beaconkey:
                continue
            devices.append(
                {
                    "did": did,
                    "mac": (device.get("mac") or "").upper(),
                    "name": device.get("name"),
                    "model": device.get("model"),
                    "token": device.get("token"),
                    "beaconkey": beaconkey,
                    "server": server,
                }
            )
        return devices

    def _read_cache(self) -> list[dict] | None:
        try:
            with io.open(self._storage, encoding="utf8") as f:
                return json.load(f)
        except (OSError, ValueError):
            return None

    def _write_cache(self, devices: list[dict]):
        try:
            self._storage.parent.mkdir(parents=True, exist_ok=True)
            with io.open(self._storage, "w", encoding="utf8") as f:
                json.dump(devices, f, ensure_ascii=False, indent=2)
        except OSError as err:
            _LOGGER.error("Unable to store Xiaomi devices to %s: %s", self._storage, err)

    def _cache_expired(self) -> bool:
        return (time.time() - self._storage.stat().st_mtime) > self._update_interval

    def _load_devices(self) -> list[dict]:
        cached = self._read_cache()
        if cached is not None and not self._cache_expired():
            return cached

        cloud = self._login()
        if cloud is None:
            if cached is None:
                return []
            _LOGGER.warning(
                "Using outdated Xiaomi devices cache, next login attempt in %s seconds",
                int(self._update_interval),
            )
            # postpone next login attempt to not block every build
            os.utime(self._storage)
            return cached

        devices = []
        loaded = False
        for server in self._servers:
            try:
                devices.extend(self._fetch_devices(cloud, server))
                loaded = True
            except Exception as err:
                _LOGGER.error("Unable to get devices from server %s: %s", server, err)
                # keep previously loaded devices of failed server
                devices.extend(d for d in cached or [] if d.get("server") == server)

        if not loaded:
            return cached or []

        self._write_cache(devices)
        return devices

    def get_devices(self) -> list[dict]:
        # load only once per run to not ask for login for every device
        if self._devices is None:
            self._devices = self._load_devices()
        return self._devices

    def _get(self, mac: str, key: str):
        mac = str(mac).upper()
        found = [device for device in self.get_devices() if device["mac"] == mac]
        if len(found) == 0:
            return None
        if len(found) > 1:
            _LOGGER.warning(
                'More than one server configured for the same device %s. Using "%s" server. This may lead to not to decode beacons.',
                mac,
                found[0]["server"],
            )
        return found[0][key]

    # DEPRECATED
    def get(self, mac: str):
        return self.get_beaconkey(mac)

    def get_beaconkey(self, mac: str) -> str:
        return self._get(mac, "beaconkey")

    def get_token(self, mac: str) -> str:
        return self._get(mac, "token")
