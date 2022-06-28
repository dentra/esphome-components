import json
import io
import pathlib
import time
import logging
from pathlib import Path
from .xiaomi_cloud_connecor import XiaomiCloudConnector

_LOGGER = logging.getLogger(__name__)


class XiaomiAccount:
    def __init__(
        self,
        username: str,
        password: str,
        servers: list[str],
        storage_path: str = ".",
        update_interval: float = 60 * 60,
    ):
        self._connector = XiaomiCloudConnector(username, password)
        self._servers = servers
        self._storage = f"{storage_path}/.xiaomi-devices.json"
        self._update_interval = update_interval
        self._devices = []

    def login_(self):
        return self._connector.login()

    def process_server_(self, server: str):
        _LOGGER.debug("Processing server %s", server)
        devices = self._connector.get_devices(server)
        if devices is None:
            _LOGGER.error("Unable to get devices from server %s", server)
            return

        for device in devices["result"]["list"]:
            self.process_device_(server, device)

    def process_device_(self, server: str, device: dict):
        _LOGGER.debug('Processing device "%s"', device.get("name"))
        if "did" not in device:
            return
        if "blt" not in device["did"]:
            return
        beaconkey = self._connector.get_beaconkey(server, device["did"])
        if not beaconkey:
            return
        self._devices.append(
            {
                "did": device["did"],
                "mac": device.get("mac").upper(),
                "name": device.get("name"),
                "model": device.get("model"),
                "token": device.get("token"),
                "beaconkey": beaconkey.get("result", {}).get("beaconkey"),
                "server": server,
            }
        )

    def load_(self):
        _LOGGER.debug("Login into Xiaomi account %s", self._connector._username)
        if not self._connector.login():
            _LOGGER.error(
                "Unable to login into Xiaomi account %s", self._connector._username
            )
            return

        for server in self._servers:
            self.process_server_(server)

        pathlib.Path(self._storage).parent.mkdir(parents=True, exist_ok=True)

        with io.open(self._storage, "w", encoding="utf8") as f:
            json.dump(self._devices, f, ensure_ascii=False, indent=2)

    def read_(self):
        with io.open(self._storage, encoding="utf8") as f:
            self._devices = json.load(f)

    def requires_load_(self):
        path = Path(self._storage)
        if not path.is_file():
            return True
        return (time.time() - path.stat().st_mtime) > self._update_interval

    def get_devices(self) -> list[dict]:
        if self.requires_load_():
            self.load_()
        else:
            self.read_()
        return self._devices

    def _get(self, mac: str, key: str):
        devices = self.get_devices()
        mac = str(mac).upper()
        found: list[dict] = []
        for device in devices:
            if device["mac"] == mac:
                found.append(device)
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
