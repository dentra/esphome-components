FROM esphome/esphome-lint:dev

ARG esphome_version
ARG apt_deps

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get update \
 && apt-get install -y --no-install-recommends sudo $apt_deps \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# pyparsing needed to fix https://github.com/esphome/issues/issues/2540
RUN pip3 --disable-pip-version-check --no-cache-dir install esphome==$esphome_version pyparsing==2.2.0 \
 && rm -rf /tmp/pip-tmp

RUN echo "alias ll='ls -l'" >> /etc/bash.bashrc \
 && echo "alias la='ls -A'" >> /etc/bash.bashrc \
 && echo "alias l='ls -CF'" >> /etc/bash.bashrc

RUN useradd -m -s /bin/bash -G sudo vscode \
 && echo "vscode ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/vscode \
 && chmod 0440 /etc/sudoers.d/vscode

RUN mv /root/.platformio /home/vscode && chown -R vscode:vscode /home/vscode/.platformio /esphome /piolibs

