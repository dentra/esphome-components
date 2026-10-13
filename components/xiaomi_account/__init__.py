import logging
import re
from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE
from esphome.const import (
    CONF_BINDKEY,
    CONF_NAME,
    CONF_USERNAME,
    CONF_PASSWORD,
    CONF_SERVERS,
    CONF_UPDATE_INTERVAL,
)
from .xiaomi_account import XiaomiAccount

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@dentra"]

CONF_XIAOMI_ACCOUNT = "xiaomi_account"
CONF_TOKEN = "token"

DEFAULT_SERVERS = ["cn", "de", "us", "ru", "tw", "sg", "in", "i2"]

# cv.sensitive is not available in older ESPHome versions
_sensitive = getattr(cv, "sensitive", lambda inner: inner)


def _deprecated_password(config):
    if CONF_PASSWORD in config:
        _LOGGER.warning(
            "%s.%s is deprecated and ignored, login is performed with QR code",
            CONF_XIAOMI_ACCOUNT,
            CONF_PASSWORD,
        )
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            # used only to separate stored login sessions of different accounts
            cv.Optional(CONF_USERNAME): cv.string_strict,
            cv.Optional(CONF_PASSWORD): _sensitive(cv.string_strict),
            cv.Optional(CONF_UPDATE_INTERVAL, default="1d"): cv.update_interval,
            cv.Optional(CONF_SERVERS, default=DEFAULT_SERVERS): cv.All(
                cv.ensure_list(cv.one_of(*DEFAULT_SERVERS, lower=True)),
                cv.Length(min=1),
            ),
        }
    ),
    _deprecated_password,
)


def cv_bindkey(value):
    value = cv.string_strict(value)
    parts = [value[i : i + 2] for i in range(0, len(value), 2)]
    if len(parts) != 16 and len(parts) != 12:
        raise cv.Invalid("Bind key must consist of 16 or 12 hexadecimal numbers")
    parts_int = []
    if any(len(part) != 2 for part in parts):
        raise cv.Invalid("Bind key must be format XX")
    for part in parts:
        try:
            parts_int.append(int(part, 16))
        except ValueError:
            # pylint: disable=raise-missing-from
            raise cv.Invalid("Bind key must be hex values from 00 to FF")
    if len(parts_int) == 12:
        parts_int.extend([0xFF, 0xFF, 0xFF, 0xFF])

    return "".join(f"{part:02X}" for part in parts_int)


def cv_token(value):
    value = cv.string_strict(value)
    parts = [value[i : i + 2] for i in range(0, len(value), 2)]
    if len(parts) != 12:
        raise cv.Invalid("Token must consist of 12 hexadecimal numbers")
    parts_int = []
    if any(len(part) != 2 for part in parts):
        raise cv.Invalid("Token must be format XX")
    for part in parts:
        try:
            parts_int.append(int(part, 16))
        except ValueError:
            # pylint: disable=raise-missing-from
            raise cv.Invalid("Token must be hex values from 00 to FF")

    return "".join(f"{part:02X}" for part in parts_int)


XIAOMI_ACCOUNT_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_XIAOMI_ACCOUNT): CONFIG_SCHEMA,
    }
)

BINDKEY_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_BINDKEY): cv_bindkey,
    }
)

TOKEN_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_TOKEN): cv_token,
    }
)


def as_token(value):
    cpp_array = [
        f"0x{part}" for part in [value[i : i + 2] for i in range(0, len(value), 2)]
    ]
    return cg.RawExpression("(const uint8_t[12]){{{}}}".format(",".join(cpp_array)))


def as_bindkey(value):
    cpp_array = [
        f"0x{part}" for part in [value[i : i + 2] for i in range(0, len(value), 2)]
    ]
    return cg.RawExpression("(const uint8_t[16]){{{}}}".format(",".join(cpp_array)))


_XIAOMI_ACCOUNT: XiaomiAccount = None


def _get_xiaomi_account() -> XiaomiAccount:
    global _XIAOMI_ACCOUNT
    if _XIAOMI_ACCOUNT is not None:
        return _XIAOMI_ACCOUNT

    conf = CORE.config.get("miot")
    if conf is not None:
        conf = conf.get(CONF_XIAOMI_ACCOUNT)
    if conf is None:
        conf = CORE.config.get(CONF_XIAOMI_ACCOUNT)
    if conf is None:
        return

    session_name = re.sub(r"[^\w.@-]", "_", conf.get(CONF_USERNAME, "default"))
    _XIAOMI_ACCOUNT = XiaomiAccount(
        servers=conf[CONF_SERVERS],
        session_file=Path(CORE.data_dir) / CONF_XIAOMI_ACCOUNT / f"{session_name}.json",
        storage_path=CORE.build_path,
        update_interval=conf[CONF_UPDATE_INTERVAL].total_seconds,
        # dashboard log has gaps between lines and themed colors, text QR is unreadable there
        render_qr=not CORE.dashboard,
    )

    return _XIAOMI_ACCOUNT


def set_bindkey(config, mac, setter):
    if CONF_BINDKEY in config:
        cg.add(setter(as_bindkey(config[CONF_BINDKEY])))
    else:
        xa = _get_xiaomi_account()
        if xa is None:
            _LOGGER.warning(
                "%s not configured for %s %s",
                CONF_XIAOMI_ACCOUNT,
                config.get(CONF_NAME, ""),
                CONF_BINDKEY,
            )
            return

        bindkey = xa.get_beaconkey(mac)
        if bindkey:
            _LOGGER.info("Got bindkey for %s %s", mac, config.get(CONF_NAME, ""))
            cg.add(setter(as_bindkey(bindkey)))
        else:
            _LOGGER.error("No bindkey found for %s %s", mac, config.get(CONF_NAME, ""))


def set_token(config, mac, setter):
    if CONF_TOKEN in config:
        cg.add(setter(as_bindkey(config[CONF_TOKEN])))
    else:
        xa = _get_xiaomi_account()
        if xa is None:
            _LOGGER.warning(
                "%s not configured for %s %s",
                CONF_XIAOMI_ACCOUNT,
                config.get(CONF_NAME, ""),
                CONF_TOKEN,
            )
            return

        token = xa.get_token(mac)
        if token:
            _LOGGER.info("Got token for %s %s", mac, config.get(CONF_NAME, ""))
            cg.add(setter(as_token(token)))
        else:
            _LOGGER.error("No token found for %s %s", mac, config.get(CONF_NAME, ""))
