import json
import io
import time
import logging
from pathlib import Path
from .xiaomi_cloud_connecor import XiaomiCloudConnector

_LOGGER = logging.getLogger(__name__)


class XiaomiBeaconkeys:
    def __init__(
        self,
        username: str,
        password: str,
        servers: list,
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

    def process_server_(self, server):
        _LOGGER.debug(f"Processing server {server}")
        devices = self._connector.get_devices(server)
        if devices is None:
            _LOGGER.error(f"Unable to get devices from server {server}")
            return

        for device in devices["result"]["list"]:
            self.process_device_(server, device)

    def process_device_(self, server, device):
        _LOGGER.debug(f"Processing device \"{device.get('name')}\"")
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
            }
        )

    def load_(self):
        _LOGGER.debug(f"Login into Xiaomi account {self._connector._username}")
        if not self._connector.login():
            _LOGGER.error(
                f"Unable to login into Xiaomi account {self._connector._username}"
            )
            return

        for server in self._servers:
            self.process_server_(server)

        with io.open(self._storage, "w", encoding="utf8") as f:
            json.dump(self._devices, f, ensure_ascii=False, indent=2)

    def read_(self):
        with io.open(self._storage) as f:
            self._devices = json.load(f)

    def requires_load_(self):
        path = Path(self._storage)
        if not path.is_file():
            return True
        return (time.time() - path.stat().st_mtime) > self._update_interval

    def get_devices(self):
        if self.requires_load_():
            self.load_()
        else:
            self.read_()
        return self._devices

    def get(self, mac: str):
        self.get_devices()
        mac = str(mac).upper()
        for device in self._devices:
            if device["mac"] == mac:
                return device["beaconkey"]
        return None
